#include "Assets/MaterialManager.h"

#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{
	FPipelineBuilder FMaterialManager::CreateOpaquePipeline(std::string_view shaderName)
	{
		FPipelineBuilder pipelineBuilder = {};
		pipelineBuilder
			.SetPushConstantType<FMaterial::PushConstants>()
			.SetName(FName(fmt::format("Material_{}", shaderName)));

		pipelineBuilder.GetShaderState()
			.AddStage(shaderName, vk::ShaderStageFlagBits::eVertex)
			.AddStage(shaderName, vk::ShaderStageFlagBits::eFragment);

		pipelineBuilder.GetBlendState()
			.AddNoBlendingState();

		pipelineBuilder.GetDepthStencil()
			.SetDepth(true, true, vk::CompareOp::eGreaterOrEqual);

		pipelineBuilder.GetPipelineRendering()
			.AddColorAttachment(FGeometryBuffer::kColorFormat)
			.SetDepthAttachment(FGeometryBuffer::kDepthFormat);

		return pipelineBuilder;
	}

	THandle<FMaterial> FMaterialManager::LoadMaterial(const FPipelineBuilder& pipelineBuilder, size_t uniformStructSize, size_t maxInstances)
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		const THandle<FPipeline> pipelineHandle = gpu.CreatePipeline(pipelineBuilder);
		TURBO_CHECK(pipelineHandle);

		const THandle<FMaterial> materialHandle = mMaterialPool.Acquire();
		FMaterial* material = mMaterialPool.Access(materialHandle);
		material->mPipeline = pipelineHandle;
		material->mUniformBuffer = {};
		material->mUniformStructSize = uniformStructSize;
		material->mMaxInstances = maxInstances;

		if (uniformStructSize > 0)
		{
			FBufferBuilder bufferBuilder = {};
			bufferBuilder
				.Init(vk::BufferUsageFlagBits::eUniformBuffer, EBufferFlags::None, uniformStructSize * maxInstances)
				.SetName(FName(fmt::format("{}_Uniforms", pipelineBuilder.GetName())));
			material->mUniformBuffer = gpu.CreateBuffer(bufferBuilder);
			TURBO_CHECK(material->mUniformBuffer);
		}

		mMaterialToMaterialInstanceMap[materialHandle] = FMaterialInstanceArray();

		FAvailableIndexes availableIndexes = FAvailableIndexes();
		availableIndexes.resize(maxInstances);

		for (size_t instanceId = 0; instanceId < maxInstances; ++instanceId)
		{
			availableIndexes[instanceId] = maxInstances - instanceId - 1;
		}

		mMaterialToAvailableIndexesMap[materialHandle] = std::move(availableIndexes);

		return materialHandle;
	}

	THandle<FMaterial::Instance> FMaterialManager::CreateMaterialInstance(THandle<FMaterial> materialHandle)
	{
		FMaterial* material = AccessMaterial(materialHandle);
		TURBO_CHECK(material)

		FAvailableIndexes& availableIndexes = mMaterialToAvailableIndexesMap.at(materialHandle);
		TURBO_CHECK(!availableIndexes.empty())

		THandle<FMaterial::Instance> instanceHandle = mMaterialInstancePool.Acquire();
		FMaterial::Instance* instance = mMaterialInstancePool.Access(instanceHandle);

		instance->material = materialHandle;
		instance->mUniformBufferIndex = availableIndexes.back();
		availableIndexes.pop_back();

		return instanceHandle;
	}

	void FMaterialManager::UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, std::span<byte> data)
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		const FMaterial::Instance* instance = mMaterialInstancePool.Access(instanceHandle);
		const FMaterial* material = mMaterialPool.Access(instance->material);

		TURBO_CHECK(data.size() == material->mUniformStructSize)

		const FBufferBuilder bufferBuilder = FBufferBuilder::CreateStagingBuffer(data);
		const THandle<FBuffer> stagingBufferHandle = gpu.CreateBuffer(bufferBuilder);
		FBuffer* stagingBuffer = gpu.AccessBuffer(stagingBufferHandle);

		std::memcpy(stagingBuffer->GetMappedAddress(), data.data(), data.size());

		cmd.BufferBarrier(
			stagingBufferHandle,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eTransferRead,
			vk::PipelineStageFlagBits2::eTransfer
			);

		const FCopyBufferInfo copyBufferInfo = {
			.mSrc = stagingBufferHandle,
			.mSrcOffset = 0,
			.mDst = material->mUniformBuffer,
			.mDstOffset = material->mUniformStructSize * instance->mUniformBufferIndex,
			.mSize = material->mUniformStructSize
		};

		cmd.CopyBuffer(copyBufferInfo);
		cmd.BufferBarrier(
			material->mUniformBuffer,
			vk::AccessFlagBits2::eTransferWrite,
			vk::PipelineStageFlagBits2::eTransfer,
			vk::AccessFlagBits2::eUniformRead,
			vk::PipelineStageFlagBits2::eVertexShader
			);

		gpu.DestroyBuffer(stagingBufferHandle);
	}

	FDeviceAddress FMaterialManager::GetMaterialInstanceAddress(const FGPUDevice& gpu, THandle<FMaterial::Instance> instanceHandle) const
	{
		const FMaterial::Instance* instance = mMaterialInstancePool.Access(instanceHandle);
		const FMaterial* material = mMaterialPool.Access(instance->material);
		if (material->mUniformBuffer.IsValid())
		{
			const FBuffer* uniformBuffer = gpu.AccessBuffer(material->mUniformBuffer);
			return uniformBuffer->GetDeviceAddress() + instance->mUniformBufferIndex * material->mUniformStructSize;
		}

		return kNullDeviceAddress;
	}

	void FMaterialManager::DestroyMaterial(THandle<FMaterial> materialHandle)
	{
		TRACE_ZONE_SCOPED()

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		const FMaterial* material = mMaterialPool.Access(materialHandle);
		TURBO_CHECK(material);
		gpu.DestroyPipeline(material->mPipeline);

		if (material->mUniformBuffer.IsValid())
		{
			gpu.DestroyBuffer(material->mUniformBuffer);
		}

		const FMaterialInstanceArray& instanceHandles = mMaterialToMaterialInstanceMap.at(materialHandle);
		for (const THandle materialInstanceHandle : instanceHandles)
		{
			mMaterialInstancePool.Release(materialInstanceHandle);
		}

		mMaterialToMaterialInstanceMap.erase(materialHandle);
		mMaterialToAvailableIndexesMap.erase(materialHandle);
	}

	void FMaterialManager::DestroyMaterialInstance(THandle<FMaterial::Instance> handle)
	{
		FMaterial::Instance* materialInstance = mMaterialInstancePool.Access(handle);
		TURBO_CHECK(materialInstance);

		FAvailableIndexes& availableIndexes = mMaterialToAvailableIndexesMap.at(materialInstance->material);
		availableIndexes.push_back(materialInstance->mUniformBufferIndex);

		FMaterialInstanceArray& instanceHandles = mMaterialToMaterialInstanceMap.at(materialInstance->material);
		instanceHandles.erase(handle);

		mMaterialInstancePool.Release(handle);
	}
}
