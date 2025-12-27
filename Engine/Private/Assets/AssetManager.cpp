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
	void FOldAssetManager::Init(FGPUDevice& gpu)
	{
	}

	void FOldAssetManager::Destroy(FGPUDevice& gpu) const
	{
	}

	THandle<FTexture> FOldAssetManager::LoadTexture(const std::filesystem::path& path, bool bSRGB)
	{
		THandle<FTexture> result = {};

		if (path.extension() == ".dds")
		{
			result = LoadDDS(path, bSRGB);
		}

		return result;
	}

	THandle<FTexture> FOldAssetManager::LoadDDS(const std::filesystem::path& path, bool bSRGB)
	{
		TRACE_ZONE_SCOPED()

		THandle<FTexture> result = {};
		TURBO_LOG(LogTextureLoading, Info, "Loading {} using DDS loader.", path.c_str());

		dds::Image image;
		if (const dds::ReadResult readResult = dds::readFile(path, &image);
			readResult != dds::ReadResult::Success)
		{
			TURBO_LOG(LogTextureLoading, Error, "Error {} during loading {}", magic_enum::enum_name(readResult), path.c_str());
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
			TURBO_LOG(LogTextureLoading, Error, "Invalid texture type for {}", path.c_str());
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

		// Create staging buffer
		const FBufferBuilder stagingBufferBuilder = FBufferBuilder::CreateStagingBuffer(numDataBytes);

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