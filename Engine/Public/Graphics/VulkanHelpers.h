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

		constexpr uint32 CalculateImageSize(glm::int2 size, vk::Format format)
		{
			const uint32 numPixels = size.x * size.y;

			if (format > vk::Format::eR8Unorm && format <= vk::Format::eR8Srgb)
			{
				// 1bpp
				return numPixels;
			}
			else if (format > vk::Format::eR8G8Unorm && format <= vk::Format::eR8G8Srgb)
			{
				// 2bpp
				return numPixels * 2;
			}
			else if (format > vk::Format::eR8G8B8Unorm && format <= vk::Format::eR8G8B8Srgb)
			{
				// 3bpp
				return numPixels * 3;
			}
			else if (format > vk::Format::eR8G8B8A8Unorm && format <= vk::Format::eB8G8R8A8Srgb)
			{
				// 4bpp
				return numPixels * 4;
			}

			// BC compression
			else if (format > vk::Format::eBc1RgbUnormBlock && format <= vk::Format::eBc1RgbaSrgbBlock)
			{
				return numPixels / 2;
			}
			else if (format > vk::Format::eBc3UnormBlock && format <= vk::Format::eBc3SrgbBlock)
			{
				return numPixels;
			}
			else if (format > vk::Format::eBc4UnormBlock && format <= vk::Format::eBc4SnormBlock)
			{
				return numPixels / 2;
			}
			else if (format > vk::Format::eBc5UnormBlock && format <= vk::Format::eBc5SnormBlock)
			{
				return numPixels;
			}
			else if (format > vk::Format::eBc6HUfloatBlock && format <= vk::Format::eBc6HSfloatBlock)
			{
				return numPixels;
			}
			else if (format > vk::Format::eBc7UnormBlock && format <= vk::Format::eBc7SrgbBlock)
			{
				return numPixels;
			}

			TURBO_UNINPLEMENTED();
			return 0;
		}
	}

	namespace VulkanConverters
	{
		inline vk::Extent2D ToExtent2D(const glm::uvec2& size) { return {size.x, size.y}; }
		inline vk::Extent3D ToExtent3D(const glm::uvec3& size) { return {size.x, size.y, size.z}; }

		inline vk::Offset2D ToOffset2D(const glm::ivec2& size) { return vk::Offset2D{size.x, size.y}; }
		inline vk::Offset3D ToOffset3D(const glm::ivec3& size) { return vk::Offset3D{size.x, size.y, size.z}; }
	}

	constexpr vk::ColorComponentFlags kRGBABits =
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
		| vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	constexpr vk::ColorComponentFlags kRGBBits =
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
		| vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	namespace VulkanEnum
	{
		inline cstring GetShaderStageName(vk::ShaderStageFlagBits shaderStage)
		{
			switch (shaderStage)
			{
			case vk::ShaderStageFlagBits::eVertex:
				return "Vertex";
			case vk::ShaderStageFlagBits::eTessellationControl:
				return "TessellationControl";
			case vk::ShaderStageFlagBits::eTessellationEvaluation:
				return "TessellationEvaluation";
			case vk::ShaderStageFlagBits::eGeometry:
				return "Geometry";
			case vk::ShaderStageFlagBits::eFragment:
				return "Fragment";
			case vk::ShaderStageFlagBits::eCompute:
				return "Compute";
			case vk::ShaderStageFlagBits::eRaygenKHR:
				return "ReyGen";
			case vk::ShaderStageFlagBits::eAnyHitKHR:
				return "AnyHit";
			case vk::ShaderStageFlagBits::eClosestHitKHR:
				return "ClosestHit";
			case vk::ShaderStageFlagBits::eMissKHR:
				return "Miss";
			case vk::ShaderStageFlagBits::eIntersectionKHR:
				return "Intersection";
			case vk::ShaderStageFlagBits::eCallableKHR:
				return "Callable";
			case vk::ShaderStageFlagBits::eTaskEXT:
				return "Task";
			case vk::ShaderStageFlagBits::eMeshEXT:
				return "Mesh";
			default: ;
			}

			return "Unknown";
		}
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

	template <>
	struct HandleTraits<vk::Buffer>
	{
		static uint64 CastToU64Handle(vk::Buffer handle)
		{
			VkBuffer nativeHandle = handle;
			return reinterpret_cast<uint64>(nativeHandle);
		}

		static cstring GetTypePostFix()
		{
			return "_Buffer";
		}
	};

	template <>
	struct HandleTraits<vk::Pipeline>
	{
		static uint64 CastToU64Handle(vk::Pipeline handle)
		{
			VkPipeline nativeHandle = handle;
			return reinterpret_cast<uint64>(nativeHandle);
		}

		static cstring GetTypePostFix()
		{
			return "_Pipeline";
		}
	};

	template <>
	struct HandleTraits<vk::Sampler>
	{
		static uint64 CastToU64Handle(vk::Sampler handle)
		{
			VkSampler nativeHandle = handle;
			return reinterpret_cast<uint64>(nativeHandle);
		}

		static cstring GetTypePostFix()
		{
			return "_Sampler";
		}
	};
}
