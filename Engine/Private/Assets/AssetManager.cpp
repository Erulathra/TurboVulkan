#include "Assets/AssetManager.h"

#include "Assets/StaticMesh.h"
#include "Core/Engine.h"
#include "Core/FileSystem.h"
#include "Graphics/GPUDevice.h"

#include "fastgltf/core.hpp"
#include "fastgltf/glm_element_traits.hpp"
#include "fastgltf/tools.hpp"

#include "dds.hpp"
#include "Assets/GLTFHelpers.h"
#include "World/World.h"

namespace Turbo
{
	constexpr std::string_view kPositionName = "POSITION";
	constexpr std::string_view kNormalName = "NORMAL";
	constexpr std::string_view kUVName = "TEXCOORD_0";
	constexpr std::string_view kColorName = "COLOR_0";

	void LogGLTFError(std::string_view message, fastgltf::Error error)
	{
		TURBO_LOG(LogMeshLoading, Error, "{} Error: {} Message: {}", message, fastgltf::getErrorName(error), fastgltf::getErrorMessage(error));
	}

	template<typename T>
	void LoadComponentBuffer( const fastgltf::Asset& meshAsset, THandle<FBuffer>& outBuffer, FDeviceAddress& outDeviceAddress, std::string_view attributeName)
	{
		const fastgltf::Mesh& gltfMesh = meshAsset.meshes.front();
		const fastgltf::Primitive& gltfSubmesh = gltfMesh.primitives.front();

		if (auto attribute = gltfSubmesh.findAttribute(attributeName);
			attribute != gltfSubmesh.attributes.cend())
		{
			const fastgltf::Accessor& accessor = meshAsset.accessors[attribute->accessorIndex];

			std::vector<T> componentData;
			componentData.reserve(accessor.count);

			fastgltf::iterateAccessor<T>(meshAsset, accessor, [&](const T& vertex)
			{
				componentData.push_back(vertex);
			});

			FBufferBuilder bufferBuilder = {};
			bufferBuilder.Init(
				vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
				EBufferFlags::None,
				componentData.size() * sizeof(T)
			);
			bufferBuilder.SetData(componentData.data());
			bufferBuilder.SetName(FName(fmt::format("{}_{}", gltfMesh.name, attributeName)));

			FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
			outBuffer = gpu.CreateBuffer(bufferBuilder);
			FBuffer* outBufferData = gpu.AccessBuffer(outBuffer);
			outDeviceAddress = outBufferData->GetDeviceAddress();
		}
	}

	void FAssetManager::Init(FGPUDevice& gpu)
	{
		FBufferBuilder bufferBuilder = {};
		bufferBuilder
			.Init(vk::BufferUsageFlagBits::eUniformBuffer, EBufferFlags::None, sizeof(FMeshPointers) * kMaxMeshes)
			.SetName(FName("MeshPointer"));

		mMeshPointersPool = gpu.CreateBuffer(bufferBuilder);
	}

	void FAssetManager::Destroy(FGPUDevice& gpu) const
	{
		gpu.DestroyBuffer(mMeshPointersPool);
	}

	THandle<FMesh> FAssetManager::LoadMesh(FName path, bool bLevelAsset)
	{
		if (THandle<FMesh> cachedAsset = FindCachedAsset<FMesh>(path))
		{
			return cachedAsset;
		}

		// This method is as much naive as it can be, but for now it should last.
		TURBO_LOG(LogMeshLoading, Info, "Loading GLTF mesh. ({})", path.ToString())

		std::vector<byte> meshBytes;
		FileSystem::LoadAssetData(path, meshBytes);

		FTurboGLTFDataBuffer dataBuffer = FTurboGLTFDataBuffer::Load(path);

		fastgltf::Parser parser;
		fastgltf::Expected<fastgltf::Asset> meshAsset = parser.loadGltf(dataBuffer, path.ToCString(), fastgltf::Options::GenerateMeshIndices);
		if (meshAsset.error() != fastgltf::Error::None)
		{
			LogGLTFError("Parsing error.", meshAsset.error());
			return {};
		}

		TURBO_CHECK_MSG(meshAsset->meshes.size() == 1, "Only one mesh per file is supported");
		TURBO_CHECK_MSG(meshAsset->meshes.front().primitives.size() == 1, "Only one submesh is supported");

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		fastgltf::Mesh& gltfMesh = meshAsset->meshes.front();
		const uint32 subMeshId = 0;

		THandle<FMesh> meshHandle = mMeshPool.Acquire();
		FMesh* subMesh = mMeshPool.Access(meshHandle);
		subMesh->mName = path;

		fastgltf::Primitive& glftSubMesh = gltfMesh.primitives[subMeshId];

		TURBO_CHECK(glftSubMesh.indicesAccessor)
		{
			const fastgltf::Accessor& indicesAccessor = meshAsset->accessors[glftSubMesh.indicesAccessor.value()];

			std::vector<uint32> indices;
			indices.reserve(indicesAccessor.count);
			subMesh->mVertexCount = indicesAccessor.count;

			fastgltf::iterateAccessor<uint32>(meshAsset.get(), indicesAccessor, [&](uint32 index)
			{
				indices.push_back(index);
			});

			FBufferBuilder bufferBuilder = {};
			bufferBuilder.Init(
				vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				EBufferFlags::None,
				indices.size() * sizeof(uint32)
			);
			bufferBuilder.SetData(indices.data());
			bufferBuilder.SetName(FName(fmt::format("{}_INDICES", meshAsset->meshes.front().name)));

			subMesh->mIndicesBuffer = gpu.CreateBuffer(bufferBuilder);
		}

		FMeshPointers meshPointers = {};

		LoadComponentBuffer<glm::vec3>(meshAsset.get(), subMesh->mPositionBuffer, meshPointers.mPositionBuffer, kPositionName);
		LoadComponentBuffer<glm::vec3>(meshAsset.get(), subMesh->mNormalBuffer, meshPointers.mNormalBuffer, kNormalName);
		LoadComponentBuffer<glm::vec2>(meshAsset.get(), subMesh->mUVBuffer, meshPointers.mUVBuffer, kUVName);
		LoadComponentBuffer<glm::vec4>(meshAsset.get(), subMesh->mColorBuffer, meshPointers.mColorBuffer, kColorName);

		gpu.ImmediateSubmit(FOnImmediateSubmit::CreateLambda([&](FCommandBuffer& cmd)
		{
			const FBufferBuilder stagingBufferBuilder = FBufferBuilder::CreateStagingBuffer(&meshPointers, sizeof(FMeshPointers));
			const THandle<FBuffer> stagingBuffer = gpu.CreateBuffer(stagingBufferBuilder);
			cmd.BufferBarrier(
				stagingBuffer,
				vk::AccessFlagBits2::eHostWrite,
				vk::PipelineStageFlagBits2::eHost,
				vk::AccessFlagBits2::eTransferRead,
				vk::PipelineStageFlagBits2::eTransfer
				);

			const FCopyBufferInfo copyBufferInfo = {
				.mSrc = stagingBuffer,
				.mDst = mMeshPointersPool,
				.mDstOffset = sizeof(FMeshPointers) * meshHandle.GetIndex(),
				.mSize = sizeof(FMeshPointers)
			};

			cmd.CopyBuffer(copyBufferInfo);

			gpu.DestroyBuffer(stagingBuffer);
		}));

		if (bLevelAsset)
		{
			gEngine->GetWorld()->mRuntimeLevel.mLoadedMeshes.push_back(meshHandle);
		}

		mAssetCache[path] = meshHandle;
		return meshHandle;
	}

	FDeviceAddress FAssetManager::GetMeshPointersAddress(FGPUDevice& gpu, THandle<FMesh> handle) const
	{
		TURBO_CHECK(handle);

		const FBuffer* pointersPoolBuffer = gpu.AccessBuffer(mMeshPointersPool);
		TURBO_CHECK(pointersPoolBuffer);

		const FDeviceAddress memoryOffset = sizeof(FMeshPointers) * handle.GetIndex();
		TURBO_CHECK(memoryOffset < pointersPoolBuffer->GetSize());

		return pointersPoolBuffer->GetDeviceAddress() + memoryOffset;
	}

	void FAssetManager::UnloadMesh(THandle<FMesh> meshHandle)
	{
		const FMesh* mesh = mMeshPool.Access(meshHandle);

		if (mesh == nullptr)
		{
			return;
		}

		const std::array buffersToDestroy = {
			mesh->mIndicesBuffer,
			mesh->mPositionBuffer,
			mesh->mNormalBuffer,
			mesh->mUVBuffer,
			mesh->mColorBuffer,
		};

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		for (const THandle<FBuffer>& bufferHandle : buffersToDestroy)
		{
			if (bufferHandle.IsValid())
			{
				gpu.DestroyBuffer(bufferHandle);
			}
		}

		mAssetCache.erase(mesh->mName);
		mMeshPool.Release(meshHandle);
	}

	THandle<FTexture> FAssetManager::LoadTexture(FName path, bool bSRGB, bool bLevelAsset)
	{
		THandle<FTexture> result = {};

		if (THandle<FTexture> cachedAsset = FindCachedAsset<FTexture>(path))
		{
			return cachedAsset;
		}

		std::filesystem::path fsPath{path.ToString()};
		if (fsPath.extension() == ".dds")
		{
			result = LoadDDS(path, bSRGB);
		}

		if (result)
		{
			mAssetCache[path] = result;
		}

		if (bLevelAsset)
		{
			gEngine->GetWorld()->mRuntimeLevel.mLoadedTextures.push_back(result);
		}

		return result;
	}

	void FAssetManager::UnloadTexture(THandle<FTexture> handle)
	{
		TURBO_CHECK(handle)

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		const FTexture* texture = gpu.AccessTexture(handle);

		if (texture == nullptr)
		{
			return;
		}

		mAssetCache.erase(texture->mName);
		gpu.DestroyTexture(handle);
	}

	THandle<FTexture> FAssetManager::LoadDDS(FName path, bool bSRGB)
	{
		TRACE_ZONE_SCOPED()

		THandle<FTexture> result = {};
		TURBO_LOG(LogTextureLoading, Info, "Loading {} using DDS loader.", path.ToString());

		std::vector<byte> meshBytes;
		FileSystem::LoadAssetData(path, meshBytes);

		dds::Image image;
		if (const dds::ReadResult readResult = dds::readImage(reinterpret_cast<uint8*>(meshBytes.data()), meshBytes.size(), &image);
			readResult != dds::ReadResult::Success)
		{
			TURBO_LOG(LogTextureLoading, Error, "Error {} during loading {}", magic_enum::enum_name(readResult), path.ToString());
			return result;
		}

		vk::Format imageFormat = static_cast<vk::Format>(dds::getVulkanFormat(image.format, image.supportsAlpha));

		if (bSRGB)
		{
			switch (imageFormat)
			{
			case vk::Format::eBc1RgbUnormBlock:
				imageFormat = vk::Format::eBc1RgbSrgbBlock;
				break;
			case vk::Format::eBc1RgbaUnormBlock:
				imageFormat = vk::Format::eBc1RgbaSrgbBlock;
				break;

			case vk::Format::eBc2UnormBlock:
				imageFormat = vk::Format::eBc2SrgbBlock;
				break;

			case vk::Format::eBc3UnormBlock:
				imageFormat = vk::Format::eBc3SrgbBlock;
				break;

			case vk::Format::eBc7UnormBlock:
				imageFormat = vk::Format::eBc7SrgbBlock;
				break;

			default:
				TURBO_UNINPLEMENTED()
			}
		}

		ETextureType textureType;

		switch (image.dimension)
		{
		case dds::Texture1D:
			textureType = ETextureType::Texture1D;
			break;
		case dds::Texture2D:
			textureType = ETextureType::Texture2D;
			break;
		case dds::Texture3D:
			textureType = ETextureType::Texture3D;
			break;

		case dds::Unknown:
		case dds::Buffer:
		default:
			TURBO_LOG(LogTextureLoading, Error, "Invalid texture type for {}", path.ToString());
			return result;
		}


		FTextureBuilder textureBuilder = {};
		textureBuilder
			.Init(imageFormat, textureType)
			.SetSize(glm::ivec3(image.width, image.height, image.depth))
			.SetNumMips(image.numMips)
			.SetName(path);

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		result = gpu.CreateTexture(textureBuilder);

		// Calculate all mips size
		uint32 numDataBytes = 0;
		for (const dds::span<uint8>& mipMap : image.mipmaps)
		{
			numDataBytes += mipMap.size_bytes();
		}

		// Create staging buffer
		const FBufferBuilder stagingBufferBuilder = FBufferBuilder::CreateStagingBuffer(numDataBytes);

		const THandle<FBuffer> stagingBuffer = gpu.CreateBuffer(stagingBufferBuilder);
		void* stagingMappedAddress = gpu.AccessBuffer(stagingBuffer)->GetMappedAddress();

		// Copy data to buffer;
		uint32 dataOffset = 0;
		for (const dds::span<uint8>& mipMap : image.mipmaps)
		{
			byte* mipDataStart = static_cast<byte*>(stagingMappedAddress) + dataOffset;
			std::memcpy(mipDataStart, mipMap.data(), mipMap.size_bytes());

			dataOffset += mipMap.size_bytes();
		}

		// copy buffer to image
		gpu.ImmediateSubmit(FOnImmediateSubmit::CreateLambda(
				[&](FCommandBuffer& cmd)
				{
					cmd.TransitionImage(result, vk::ImageLayout::eTransferDstOptimal);

					uint32 bufferOffset = 0;
					for (uint32 mipIndex = 0; mipIndex < image.numMips; ++mipIndex)
					{
						cmd.CopyBufferToTexture(stagingBuffer, result, mipIndex, bufferOffset);

						const uint32 numMipBytes = image.mipmaps[mipIndex].size_bytes();
						bufferOffset += numMipBytes;
					}

					cmd.TransitionImage(result, vk::ImageLayout::eShaderReadOnlyOptimal);
				})
		);

		gpu.DestroyBuffer(stagingBuffer);

		return result;
	}
} // Turbo