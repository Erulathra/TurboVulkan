#include "Assets/AssetManager.h"

#include "Assets/StaticMesh.h"
#include "Core/Engine.h"
#include "Graphics/GPUDevice.h"

#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/glm_element_traits.hpp"

#include "dds.hpp"

namespace Turbo
{
	constexpr std::string_view kPositionName = "POSITION";
	constexpr std::string_view kNormalName = "NORMAL";
	constexpr std::string_view kUVName = "TEXCOORD_0";
	constexpr std::string_view kColorName = "COLOR_0";

	void LogGLTFError(std::string_view message, fastgltf::Error error)
	{
		TURBO_LOG(LOG_MESH_LOADING, Error, "{} Error: {} Message: {}", message, fastgltf::getErrorName(error), fastgltf::getErrorMessage(error));
	}

	template<typename T>
	void LoadComponentBuffer( const fastgltf::Asset& meshAsset, THandle<FBuffer>& buffer, std::string_view attributeName)
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
			buffer = gpu.CreateBuffer(bufferBuilder);
		}
	}

	THandle<FSubMesh> FAssetManager::LoadMesh(const std::filesystem::path& path)
	{
		// This method is as much naive as it can be, but for now it should last.
		TURBO_LOG(LOG_MESH_LOADING, Info, "Loading GLTF mesh. ({})", path.string())
		THandle<FSubMesh> Result = {};

		fastgltf::Expected<fastgltf::GltfDataBuffer> data = fastgltf::GltfDataBuffer::FromPath(path);
		if (data.error() != fastgltf::Error::None)
		{
			LogGLTFError("Loading error.", data.error());
			return Result;
		}

		fastgltf::Parser parser(fastgltf::Extensions::KHR_draco_mesh_compression);
		fastgltf::Expected<fastgltf::Asset> meshAsset = parser.loadGltf(data.get(), path, fastgltf::Options::GenerateMeshIndices);
		if (meshAsset.error() != fastgltf::Error::None)
		{
			LogGLTFError("Parsing error.", meshAsset.error());
			return Result;
		}

		TURBO_CHECK_MSG(meshAsset->meshes.size() == 1, "Only one mesh per file is supported");
		TURBO_CHECK_MSG(meshAsset->meshes.front().primitives.size() == 1, "Only one submesh is supported");

		const fastgltf::Primitive& sourceSubMesh = meshAsset->meshes.front().primitives.front();

		Result = mSubMeshPool.Acquire();
		FSubMesh* loadedSubMesh = mSubMeshPool.Access(Result);

		// Load Indices
		TURBO_CHECK(sourceSubMesh.indicesAccessor)
		{
			const fastgltf::Accessor& indicesAccessor = meshAsset->accessors[sourceSubMesh.indicesAccessor.value()];

			std::vector<uint32> indices;
			indices.reserve(indicesAccessor.count);
			loadedSubMesh->mVertexCount = indicesAccessor.count;

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

			FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
			loadedSubMesh->mIndicesBuffer = gpu.CreateBuffer(bufferBuilder);
		}

		LoadComponentBuffer<glm::vec3>(meshAsset.get(), loadedSubMesh->mPositionBuffer, kPositionName);
		LoadComponentBuffer<glm::vec3>(meshAsset.get(), loadedSubMesh->mNormalBuffer, kNormalName);
		LoadComponentBuffer<glm::vec2>(meshAsset.get(), loadedSubMesh->mUVBuffer, kUVName);
		LoadComponentBuffer<glm::vec4>(meshAsset.get(), loadedSubMesh->mColorBuffer, kColorName);

		return Result;
	}

	void FAssetManager::UnloadMesh(THandle<FSubMesh> meshToUnload)
	{
		const FSubMesh* staticMesh = mSubMeshPool.Access(meshToUnload);

		const std::array buffersToDestroy = {
			staticMesh->mIndicesBuffer,
			staticMesh->mPositionBuffer,
			staticMesh->mNormalBuffer,
			staticMesh->mUVBuffer,
			staticMesh->mColorBuffer,
		};

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		for (const THandle<FBuffer>& handle : buffersToDestroy)
		{
			if (handle.IsValid())
			{
				gpu.DestroyBuffer(handle);
			}
		}
	}

	THandle<FTexture> FAssetManager::LoadTexture(const std::filesystem::path& path, bool bSRGB)
	{
		THandle<FTexture> result = {};

		if (path.extension() == ".dds")
		{
			result = LoadDDS(path, bSRGB);
		}

		return result;
	}

	THandle<FTexture> FAssetManager::LoadDDS(const std::filesystem::path& path, bool bSRGB)
	{
		TRACE_ZONE_SCOPED()

		THandle<FTexture> result = {};
		TURBO_LOG(LOG_TEXTURE_LOADING, Info, "Loading {} using DDS loader.", path.c_str());

		dds::Image image;
		if (const dds::ReadResult readResult = dds::readFile(path, &image);
			readResult != dds::ReadResult::Success)
		{
			TURBO_LOG(LOG_TEXTURE_LOADING, Error, "Error {} during loading {}", magic_enum::enum_name(readResult), path.c_str());
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
			TURBO_LOG(LOG_TEXTURE_LOADING, Error, "Invalid texture type for {}", path.c_str());
			return result;
		}


		FTextureBuilder textureBuilder = {};
		textureBuilder
			.Init(imageFormat, textureType)
			.SetSize(glm::ivec3(image.width, image.height, image.depth))
			.SetNumMips(image.numMips)
			.SetName(FName(path.filename().string()));

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		result = gpu.CreateTexture(textureBuilder);

		// Calculate all mips size
		uint32 numDataBytes = 0;
		for (const dds::span<byte>& mipMap : image.mipmaps)
		{
			numDataBytes += mipMap.size_bytes();
		}

		const static FName kStagingBufferName = FName("TextureStagingBuffer");

		// Create staging buffer
		FBufferBuilder stagingBufferBuilder;
		stagingBufferBuilder
			.Init(vk::BufferUsageFlagBits::eTransferSrc, EBufferFlags::CreateMapped, numDataBytes)
			.SetName(kStagingBufferName);

		const THandle<FBuffer> stagingBuffer = gpu.CreateBuffer(stagingBufferBuilder);
		void* stagingMappedAddress = gpu.AccessBuffer(stagingBuffer)->GetMappedAddress();

		// Copy data to buffer;
		uint32 dataOffset = 0;
		for (const dds::span<byte>& mipMap : image.mipmaps)
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