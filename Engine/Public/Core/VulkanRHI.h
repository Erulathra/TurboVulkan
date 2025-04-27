#pragma once
#include <vulkan/vulkan_core.h>

namespace Turbo
{

	class VulkanRHI
	{
	private:
		explicit VulkanRHI();

	public:
		~VulkanRHI();

	public:
		static void Init();
		static void Destroy();

	private:
		void CreateVulkanInstance();
		void DestroyVulkanInstance();

	private:
		void EnumerateVulkanExtensions();

	private:
		static std::unique_ptr<VulkanRHI> Instance;

	private:
		struct
		{
			VkInstance Instance = VK_NULL_HANDLE;

			std::vector<VkExtensionProperties> ExtensionProperties;
		} VulkanContext;
	};

} // namespace Turbo
