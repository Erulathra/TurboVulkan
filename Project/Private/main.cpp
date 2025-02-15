#include <iostream>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "LoggingMacros.h"


int32_t main(int argc, char* argv[])
{
    TV_LOG(LOG_TEMP, LOG_ERROR, "This is error. Build: {}", BUILD_TYPE_STR);

    TV_LOG(LOG_TEMP, LOG_ERROR, "This is error");
    TV_LOG(LOG_TEMP, LOG_WARN, "This is warning");
    TV_LOG(LOG_TEMP, LOG_INFO, "This is info");

    glm::vec3 Vector = glm::vec3{1, 2, 3};
    TV_LOG(LOG_TEMP, LOG_INFO, "Vec3: {} {} {}", Vector.x, Vector.y, Vector.z);
    // Create a Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkInstance instance;
    vkCreateInstance(&createInfo, nullptr, &instance);

    // Destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);

    return 0;
}
