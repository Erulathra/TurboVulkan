#include "Assets/GLTFHelpers.h"

#include "Core/FileSystem.h"

Turbo::FTurboGLTFDataBuffer Turbo::FTurboGLTFDataBuffer::Load(FName path)
{
	FTurboGLTFDataBuffer result;
	FileSystem::LoadAssetData(path, result.mBytes);

	constexpr size_t kSimdJsonPadding = 64;
	result.mBytes.resize(result.mBytes.size() + kSimdJsonPadding);

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
