#pragma once

#include "Core/DataStructures/Pool.h"
#include "Graphics/ResourceBuilders.h"

// TODO: cache gpu device

namespace Turbo
{
	class FBuffer;
	class FPipeline;
	class FCommandBuffer;


	struct FMaterial final
	{
		using FUniformBufferIndex = uint32;
		static constexpr FUniformBufferIndex kInvalidUniformBufferIndex = std::numeric_limits<FUniformBufferIndex>::max();

		struct Instance final
		{
			THandle<FMaterial> material = {};
			uint32 mUniformBufferIndex = kInvalidUniformBufferIndex;
		};

		struct PushConstants final
		{
			glm::float4x4 mModelToProj;
			glm::float3x3 mInvModelToProj;

			DeviceAddress mViewData;
			DeviceAddress mMaterialInstance;
			DeviceAddress mMeshData;
		};

		THandle<FPipeline> mPipeline = {};
		THandle<FBuffer> mUniformBuffer = {};
		uint32 mUniformStructSize = 0;
		uint32 mMaxInstances = 0;
	};

	class FMaterialManager final
	{
		DELETE_COPY(FMaterialManager);

	public:
		static FPipelineBuilder CreateOpaquePipeline(std::string_view shaderName);

	public:
		THandle<FMaterial> LoadMaterial(const FPipelineBuilder& pipelineBuilder, size_t uniformStructSize, size_t maxInstances);

		template<typename UniformBufferStruct>
		THandle<FMaterial> LoadMaterial(FPipelineBuilder& pipelineBuilder, size_t maxInstances)
		{
			return LoadMaterial(pipelineBuilder, sizeof(UniformBufferStruct), maxInstances);
		}

		THandle<FMaterial::Instance> CreateMaterialInstance(THandle<FMaterial> materialHandle);

		void UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, std::span<byte> data);

		template<typename UniformBufferStruct>
		void UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, UniformBufferStruct* data)
		{
			byte bytes[sizeof(data)] = static_cast<byte*>(data);
			UpdateMaterialInstance(cmd, instanceHandle, bytes);
		}

	public:
		FMaterial* AccessMaterial(THandle<FMaterial> handle) { return mMaterialPool.Access(handle); }
		FMaterial::Instance* AccessInstance(THandle<FMaterial::Instance> handle) { return  mMaterialInstancePool.Access(handle); }

	public:
		void DestroyMaterial(THandle<FMaterial> materialHandle);
		void DestroyMaterialInstance(THandle<FMaterial::Instance> handle);

	private:
		TPoolGrowable<FMaterial> mMaterialPool;
		TPoolGrowable<FMaterial::Instance> mMaterialInstancePool;

		using FMaterialInstanceArray = entt::dense_set<THandle<FMaterial::Instance>>;
		using FMaterialToMaterialInstanceMap = entt::dense_map<THandle<FMaterial>, FMaterialInstanceArray>;
		FMaterialToMaterialInstanceMap mMaterialToMaterialInstanceMap;

		using FAvailableIndexes = std::vector<uint32>;
		using FMaterialToAvailableIndexes = entt::dense_map<THandle<FMaterial>, FAvailableIndexes>;
		FMaterialToAvailableIndexes mMaterialToAvailableIndexesMap;
	};

} // Turbo
