#pragma once

#include "Graphics/GraphicsCore.h"
#include "Graphics/Enums.h"

#define BUILDER_BODY()			\
	public:						\
		friend class GPUDevice;	\
	private:

namespace Turbo
{

	class FBufferBuilder final
	{
		BUILDER_BODY()

	public:
		FBufferBuilder& Reset() { mSize = 0; mInitialData = nullptr; return *this; }
		FBufferBuilder& Init(vk::BufferUsageFlags2 flags, EResourceUsageType usage, uint32 size)
			{ mUsageFlags = flags; mUsageType = usage; mSize = size; return *this; }
		FBufferBuilder& SetData(void* data) { mInitialData = data; return *this; }

		FBufferBuilder& SetName(const char* name) { mName = name; return *this; }

	private:
		vk::BufferUsageFlags2 mUsageFlags = {};
		EResourceUsageType mUsageType = EResourceUsageType::Immutable;
		uint32 mSize = 0;
		void* mInitialData = nullptr;

		const char* mName = nullptr;
	};

	class FTextureBuilder final
	{
		BUILDER_BODY()

	public:
		FTextureBuilder& Init(vk::Format format, ETextureType type, ETextureFlags flags)
			{ mFormat = format; mType = type; mFlags = flags;  return *this; }
		FTextureBuilder& SetSize(glm::ivec3 size) { mWidth = size.x; mHeight = size.y; mDepth = size.z; return *this; }
		FTextureBuilder& SetNumMips(uint8 numMips) { mNumMips = numMips; return *this; }
		FTextureBuilder& SetData(void* data) { mInitialData = data; return *this; }

		FTextureBuilder& SetName(const char* name) { mName = name; return *this; }

	private:
		void* mInitialData = nullptr;
		uint16 mWidth = 1;
		uint16 mHeight = 1;
		uint16 mDepth = 1;
		uint8 mNumMips = 1;
		ETextureFlags mFlags = ETextureFlags::Invalid;

		vk::Format mFormat = vk::Format::eUndefined;
		ETextureType mType = ETextureType::Texture2D;

		const char* mName = nullptr;
	};

	class FSamplerBuilder final
	{
		BUILDER_BODY()

	public:
		FSamplerBuilder& SetMinMagFilter(vk::Filter min, vk::Filter mag) { mMinFilter = min; mMagFilter = mag; return *this; }
		FSamplerBuilder& SetMipFilter(vk::SamplerMipmapMode filter) { mMipFilter = filter; return *this; }

		FSamplerBuilder& SetAddressU(vk::SamplerAddressMode u) { mAddressModeU = u; return *this; }
		FSamplerBuilder& SetAddressUV(vk::SamplerAddressMode u, vk::SamplerAddressMode v) { mAddressModeU = u; mAddressModeV = v; return *this; }
		FSamplerBuilder& SetAddressUVW(vk::SamplerAddressMode u, vk::SamplerAddressMode v, vk::SamplerAddressMode w)
			{ mAddressModeU = u; mAddressModeV = v; mAddressModeV = w; return *this; }

		FSamplerBuilder& SetName(const char* name) { mName = name; return *this; }

	private:
		vk::Filter mMinFilter = vk::Filter::eNearest;
		vk::Filter mMagFilter = vk::Filter::eNearest;
		vk::SamplerMipmapMode mMipFilter = vk::SamplerMipmapMode::eNearest;

		vk::SamplerAddressMode mAddressModeU = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeV = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeW = vk::SamplerAddressMode::eRepeat;

		const char* mName = nullptr;
	};

	class FShaderStateBuilder
	{
		BUILDER_BODY()
	public:
		struct ShaderStage final
		{
			std::vector<byte> mShaderData;
			vk::ShaderStageFlags mShaderStage;
		};

	public:
		FShaderStateBuilder& Reset() { mStagesCount = 0; return *this; }
		FShaderStateBuilder& AddStage(std::vector<byte>&& mShaderData, vk::ShaderStageFlags mShaderStage)
		{
			mStages[mStagesCount] = {mShaderData, mShaderStage};
			mStagesCount++;
			return *this;
		}
		FShaderStateBuilder& SetName(const char* name) { mName = name; return *this; }

	private:
		std::array<ShaderStage, kMaxShaderStages> mStages;
		uint32 mStagesCount = 0;

		const char* mName = nullptr;
	};
}

#undef BUILDER_BODY