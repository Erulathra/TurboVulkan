#include "Assets/MaterialManager.h"

#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{

	void FMaterialManager::Init(FGPUDevice& gpuDevice)
	{
		FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
		FPipelineBuilder pipelineBuilder = CreateOpaquePipeline("MeshTestMaterial.slang");
		materialManager.LoadMaterial<void, void>(FName("MeshTriangleTest"), pipelineBuilder, 0);
	}

	void FMaterialManager::Destroy(FGPUDevice& gpuDevice)
	{
		for (const auto& [key, value] : mMaterialToMaterialInstanceMap)
		{
			DestroyMaterial(key);
		}
	}

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

	THandle<FMaterial> FMaterialManager::LoadMaterial(
		FName materialName,
		const FPipelineBuilder& pipelineBuilder,
		size_t materialDataSize,
		size_t perInstanceDataSize,
		size_t maxInstances
	)
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		const THandle<FPipeline> pipelineHandle = gpu.CreatePipeline(pipelineBuilder);
		TURBO_CHECK(pipelineHandle);

		const THandle<FMaterial> materialHandle = mMaterialPool.Acquire();
		FMaterial* material = mMaterialPool.Access(materialHandle);
		material->mPipeline = pipelineHandle;
		material->mDataBuffer = {};
		material->mMaterialDataSize = materialDataSize;
		material->mPerInstanceDataSize = perInstanceDataSize;
		material->mMaxInstances = perInstanceDataSize > 0 ? maxInstances : 1;

		const size_t targetBufferSize = materialDataSize + perInstanceDataSize * maxInstances;
		if (targetBufferSize > 0)
		{
			FBufferBuilder bufferBuilder = {};
			bufferBuilder
				.Init(vk::BufferUsageFlagBits::eStorageBuffer, EBufferFlags::CreateMapped, targetBufferSize)
				.SetName(FName(fmt::format("{}_Uniforms", pipelineBuilder.GetName())));
			material->mDataBuffer = gpu.CreateBuffer(bufferBuilder);
			TURBO_CHECK(material->mDataBuffer);
		}

		mMaterialToMaterialInstanceMap[materialHandle] = FMaterialInstanceArray();

		FAvailableIndexes availableIndexes = FAvailableIndexes();
		availableIndexes.resize(maxInstances);

		for (size_t instanceId = 0; instanceId < maxInstances; ++instanceId)
		{
			availableIndexes[instanceId] = maxInstances - instanceId - 1;
		}

		mMaterialToAvailableIndexesMap[materialHandle] = std::move(availableIndexes);

		TURBO_CHECK(mMaterialNameLookUp.find(materialName) == mMaterialNameLookUp.end())
		mMaterialNameLookUp[materialName] = materialHandle;

		return materialHandle;
	}

	THandle<FMaterial> FMaterialManager::GetMaterial(FName materialName)
	{
		if (auto foundIt = mMaterialNameLookUp.find(materialName);
			foundIt != mMaterialNameLookUp.end())
		{
			return foundIt->second;
		}

		return {};
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
		TRACE_ZONE_SCOPED();
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		TRACE_GPU_SCOPED(gpu, cmd, "Update Material Instance")

		const FMaterial::Instance* instance = mMaterialInstancePool.Access(instanceHandle);
		const FMaterial* material = mMaterialPool.Access(instance->material);

		TURBO_CHECK(data.size() == material->mPerInstanceDataSize)

		const FBuffer* instancesDataBuffer = gpu.AccessBuffer(material->mDataBuffer);
		byte* targetInstanceAddress =
			static_cast<byte*>(instancesDataBuffer->GetMappedAddress()) + CalculateInstanceByteOffset(*material, instance->mUniformBufferIndex);
		std::memcpy(targetInstanceAddress, data.data(), data.size());

		cmd.BufferBarrier(
			material->mDataBuffer,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eUniformRead,
			vk::PipelineStageFlagBits2::eVertexShader
			);
	}

	FDeviceAddress FMaterialManager::GetMaterialInstanceAddress(const FGPUDevice& gpu, THandle<FMaterial::Instance> instanceHandle) const
	{
		if (const FMaterial::Instance* instance = mMaterialInstancePool.Access(instanceHandle))
		{
			if (const FMaterial* material = mMaterialPool.Access(instance->material);
				material->mDataBuffer.IsValid())
			{
				const FBuffer* uniformBuffer = gpu.AccessBuffer(material->mDataBuffer);
				return uniformBuffer->GetDeviceAddress() + CalculateInstanceByteOffset(*material, instance->mUniformBufferIndex);
			}
		}

		return kNullDeviceAddress;
	}

	void FMaterialManager::UpdateMaterialData(FCommandBuffer& cmd, THandle<FMaterial> handle, std::span<byte> data)
	{
		TRACE_ZONE_SCOPED();

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		TRACE_GPU_SCOPED(gpu, cmd, "Update Material Data")

		const FMaterial* material = mMaterialPool.Access(handle);
		TURBO_CHECK(data.size() == material->mMaterialDataSize)

		const FBuffer* instancesDataBuffer = gpu.AccessBuffer(material->mDataBuffer);
		std::memcpy(instancesDataBuffer->GetMappedAddress(), data.data(), data.size());

		cmd.BufferBarrier(
			material->mDataBuffer,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eUniformRead,
			vk::PipelineStageFlagBits2::eVertexShader
			);
	}

	FDeviceAddress FMaterialManager::GetMaterialDataAddress(const FGPUDevice& gpu, THandle<FMaterial> handle) const
	{
		const FMaterial* material = mMaterialPool.Access(handle);
		if (material->mDataBuffer.IsValid())
		{
			const FBuffer* uniformBuffer = gpu.AccessBuffer(material->mDataBuffer);
			return uniformBuffer->GetDeviceAddress();
		}

		return kNullDeviceAddress;
	}

	size_t FMaterialManager::CalculateInstanceByteOffset(const FMaterial& material, uint32 instanceIndex)
	{
		return material.mMaterialDataSize + instanceIndex * material.mPerInstanceDataSize;
	}

	void FMaterialManager::DestroyMaterial(THandle<FMaterial> materialHandle)
	{
		TRACE_ZONE_SCOPED()

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		const FMaterial* material = mMaterialPool.Access(materialHandle);
		TURBO_CHECK(material);
		gpu.DestroyPipeline(material->mPipeline);

		if (material->mDataBuffer.IsValid())
		{
			gpu.DestroyBuffer(material->mDataBuffer);
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
