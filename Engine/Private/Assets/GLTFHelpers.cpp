#include "Assets/GLTFHelpers.h"

#include "Core/FileSystem.h"

Turbo::FTurboGLTFDataBuffer Turbo::FTurboGLTFDataBuffer::Load(std::string_view path)
{
	FTurboGLTFDataBuffer result;
	FileSystem::LoadData(path, result.mBytes);

	result.mBytes.resize(result.mBytes.size() + kDataPadding);

	return result;
}

void Turbo::FTurboGLTFDataBuffer::read(void* ptr, std::size_t count)
{
	std::memcpy(ptr, mBytes.data() + readBytesNum, count);
	readBytesNum += count;
}

fastgltf::span<byte> Turbo::FTurboGLTFDataBuffer::read(std::size_t count, std::size_t padding)
{
	std::span<byte> sub(mBytes.data() + readBytesNum, count);
	readBytesNum += count;

	return sub;
}

void Turbo::FTurboGLTFDataBuffer::reset()
{
	readBytesNum = 0;
}

void Turbo::GLTF::LoadExternalBuffers(fastgltf::Asset& asset, const std::filesystem::path& assetPath)
{
	const std::filesystem::path assetDirectory = assetPath.parent_path();
	for (fastgltf::Buffer& buffer : asset.buffers)
	{
		if (std::holds_alternative<fastgltf::sources::URI>(buffer.data))
		{
			const fastgltf::sources::URI bufferURI = std::get<fastgltf::sources::URI>(buffer.data);
			const std::filesystem::path relativeBufferPath = assetDirectory / bufferURI.uri.path();

			std::vector<byte> bufferData;
			FileSystem::LoadData(relativeBufferPath.string(), bufferData);

			fastgltf::sources::Vector vectorSource = {
				std::move(bufferData)
			};

			buffer.data = std::move(vectorSource);
		}
	}
}
