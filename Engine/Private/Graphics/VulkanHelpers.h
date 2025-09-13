#pragma once

#include "Graphics/GraphicsCore.h"

namespace Turbo
{
	namespace TextureFormat
	{
		constexpr bool IsDepthStencil(vk::Format v)
		{
			return v == vk::Format::eD16UnormS8Uint
				|| v == vk::Format::eD24UnormS8Uint
				|| v == vk::Format::eD32SfloatS8Uint;
		}

		constexpr bool IsDepthOnly(vk::Format v)
		{
			return v >= vk::Format::eD16Unorm
				&& v < vk::Format::eD32Sfloat;
		}

		constexpr bool IsStencilOnly(vk::Format v)
		{
			return v == vk::Format::eS8Uint;
		}

		constexpr bool IsColor(vk::Format v)
		{
			return v > vk::Format::eUndefined && v < vk::Format::eD16Unorm;
		}

		constexpr bool HasDepth(vk::Format v)
		{
			return (v >= vk::Format::eD16Unorm && v < vk::Format::eS8Uint)
				|| (v >= vk::Format::eD16UnormS8Uint && v <= vk::Format::eD32SfloatS8Uint);
		}

		constexpr bool HasStencil(vk::Format v)
		{
			return v >= vk::Format::eS8Uint && v <= vk::Format::eD32SfloatS8Uint;
		}

		constexpr bool HasDepthOrStencil(vk::Format v)
		{
			return v >= vk::Format::eD16Unorm && v <= vk::Format::eD32SfloatS8Uint;
		}
	}

	namespace VulkanConverters
	{
		inline vk::Extent2D ToExtent2D(const glm::uvec2& size) { return {size.x, size.y}; }
		inline vk::Extent3D ToExtent3D(const glm::uvec3& size) { return {size.x, size.y, size.z}; }

		inline vk::Offset2D ToOffset2D(const glm::ivec2& size) { return vk::Offset2D{size.x, size.y}; }
		inline vk::Offset3D ToOffset3D(const glm::ivec3& size) { return vk::Offset3D{size.x, size.y, size.z}; }
	}

	template <typename HandleType>
	struct HandleTraits
	{
		static uint64 CastToU64Handle(HandleType handle)
		{
			static_assert(false, "Default CastToU64Handle is not implemented.");
			return 0;
		}

		static cstring GetTypePostFix()
		{
			static_assert(false, "Default GetTypePostFix is not implemented.");
			return "";
		}
	};

	template <>
	struct HandleTraits<vk::Image>
	{
		static uint64 CastToU64Handle(vk::Image handle)
		{
			VkImage nativeHandle = handle;
			return reinterpret_cast<uint64>(nativeHandle);
		}

		static cstring GetTypePostFix()
		{
			return "_Image";
		}
	};

	template <>
	struct HandleTraits<vk::ImageView>
	{
		static uint64 CastToU64Handle(vk::ImageView handle)
		{
			VkImageView nativeHandle = handle;
			return reinterpret_cast<uint64>(nativeHandle);
		}

		static cstring GetTypePostFix()
		{
			return "_ImageView";
		}
	};

	template <>
	struct HandleTraits<vk::ShaderModule>
	{
		static uint64 CastToU64Handle(vk::ShaderModule handle)
		{
			VkShaderModule nativeHandle = handle;
			return reinterpret_cast<uint64>(nativeHandle);
		}

		static cstring GetTypePostFix()
		{
			return "_ShaderModule";
		}
	};

	template <>
	struct HandleTraits<vk::DescriptorPool>
	{
		static uint64 CastToU64Handle(vk::DescriptorPool handle)
		{
			VkDescriptorPool nativeHandle = handle;
			return reinterpret_cast<uint64>(nativeHandle);
		}

		static cstring GetTypePostFix()
		{
			return "_DescriptorPool";
		}
	};

	template <>
	struct HandleTraits<vk::CommandBuffer>
	{
		static uint64 CastToU64Handle(vk::CommandBuffer handle)
		{
			VkCommandBuffer nativeHandle = handle;
			return reinterpret_cast<uint64>(nativeHandle);
		}

		static cstring GetTypePostFix()
		{
			return "_CommandBuffer";
		}
	};
}
