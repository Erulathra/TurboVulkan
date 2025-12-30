#pragma once

#include "Core/CoreUtils.h"
#include "Core/DataStructures/GenPoolGrowable.h"
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

		struct  Instance final
		{
			THandle<FMaterial> material = {};
			uint32 mUniformBufferIndex = kInvalidUniformBufferIndex;
		};

		struct PushConstants final
		{
			glm::float4x4 mModelToProj;
			glm::float4x4 mModelToView;
			glm::float3x3 mInvModelToView;

			FDeviceAddress mViewData;
			FDeviceAddress mMaterialData;
			FDeviceAddress mMaterialInstance;
			FDeviceAddress mMeshData;
			FDeviceAddress mSceneData;
		};

		THandle<FPipeline> mPipeline = {};
		THandle<FBuffer> mDataBuffer = {};
		uint32 mPerInstanceDataSize = 0;
		uint32 mMaterialDataSize = 0;
		uint32 mMaxInstances = 0;
	};

	class FMaterialManager final
	{
		DELETE_COPY(FMaterialManager);

	public:
		FMaterialManager() = default;

	public:
		static FPipelineBuilder CreateOpaquePipeline(std::string_view shaderName);

	public:
		template<typename MaterialDataType, typename PerInstanceDataType>
		THandle<FMaterial> LoadMaterial(FPipelineBuilder& pipelineBuilder, size_t maxInstances)
		{
			constexpr size_t materialDataSize = CoreUtils::SizeofOrZero<MaterialDataType>();
			constexpr size_t perInstanceDataSize = CoreUtils::SizeofOrZero<PerInstanceDataType>();

			return LoadMaterial(pipelineBuilder, materialDataSize, perInstanceDataSize, maxInstances);
		}

		THandle<FMaterial> LoadMaterial(
			const FPipelineBuilder& pipelineBuilder,
			size_t materialDataSize,
			size_t perInstanceDataSize,
			size_t maxInstances
		);

		THandle<FMaterial::Instance> CreateMaterialInstance(THandle<FMaterial> materialHandle);

		template<typename PerInstanceData>
		void UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, PerInstanceData* data);
		void UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, std::span<byte> data);
		[[nodiscard]] FDeviceAddress GetMaterialInstanceAddress(const FGPUDevice& gpu, THandle<FMaterial::Instance> instanceHandle) const;

		template<typename MaterialData>
		void UpdateMaterialData(FCommandBuffer& cmd, THandle<FMaterial> handle, MaterialData* data);
		auto UpdateMaterialData(FCommandBuffer& cmd, THandle<FMaterial> handle, std::span<byte> data) -> void;
		[[nodiscard]] FDeviceAddress GetMaterialDataAddress(const FGPUDevice& gpu, THandle<FMaterial> handle) const;

	public:
		[[nodiscard]] static size_t CalculateInstanceByteOffset(const FMaterial& material, uint32 instanceIndex);

	public:
		[[nodiscard]] FMaterial* AccessMaterial(THandle<FMaterial> handle) { return mMaterialPool.Access(handle); }
		[[nodiscard]] const FMaterial* AccessMaterial(THandle<FMaterial> handle) const { return mMaterialPool.Access(handle); }
		[[nodiscard]] FMaterial::Instance* AccessInstance(THandle<FMaterial::Instance> handle) { return  mMaterialInstancePool.Access(handle); }
		[[nodiscard]] const FMaterial::Instance* AccessInstance(THandle<FMaterial::Instance> handle) const { return  mMaterialInstancePool.Access(handle); }

	public:
		void DestroyMaterial(THandle<FMaterial> materialHandle);
		void DestroyMaterialInstance(THandle<FMaterial::Instance> handle);

	private:
		TGenPoolGrowable<FMaterial> mMaterialPool;
		TGenPoolGrowable<FMaterial::Instance> mMaterialInstancePool;

		using FMaterialInstanceArray = entt::dense_set<THandle<FMaterial::Instance>>;
		using FMaterialToMaterialInstanceMap = entt::dense_map<THandle<FMaterial>, FMaterialInstanceArray>;
		FMaterialToMaterialInstanceMap mMaterialToMaterialInstanceMap;

		using FAvailableIndexes = std::vector<uint32>;
		using FMaterialToAvailableIndexes = entt::dense_map<THandle<FMaterial>, FAvailableIndexes>;
		FMaterialToAvailableIndexes mMaterialToAvailableIndexesMap;
	};

	template <typename PerInstanceData>
	void FMaterialManager::UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, PerInstanceData* data)
	{
		UpdateMaterialInstance(cmd, instanceHandle, std::span<byte>(reinterpret_cast<byte*>(data), sizeof(PerInstanceData)));
	}

	template <typename MaterialData>
	void FMaterialManager::UpdateMaterialData(FCommandBuffer& cmd, THandle<FMaterial> handle, MaterialData* data)
	{
		UpdateMaterialData(cmd, handle, std::span<byte>(reinterpret_cast<byte*>(data), sizeof(MaterialData)));
	}
} // Turbo
