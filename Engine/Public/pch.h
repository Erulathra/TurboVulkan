#pragma once

// C++ standard library headers
#include <memory>
#include <utility>
#include <algorithm>
#include <ranges>
#include <functional>
#include <chrono>
#include <functional>
#include <type_traits>
#include <filesystem>
#include <string>
#include <string_view>
#include <limits>
#include <stdfloat>

// stl collections
#include <vector>
#include <span>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

// Vulkan headers
#include <vulkan/vulkan.hpp>

// VMA
#define VULKAN_HPP_ASSERT_ON_RESULT {}
#include "vk_mem_alloc.hpp"

// GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/compatibility.hpp"
#include "glm/gtx/quaternion.hpp"
#ifndef GLM_TYPE_PTR
#define GLM_TYPE_PTR
#include "glm/gtc/type_ptr.inl"
#endif // GLM_TYPE_PTR

// Magic enum
#include "magic_enum/magic_enum.hpp"

// EnTT
#include "entt/entt.hpp"

// Internal headers
#include "CommonTypeDefs.h"
#include "CommonMacros.h"
#include "CommonConstants.h"
#include "TurboLog.h"
#include "ProfilingMacros.h"
#include "Core/Name.h"
#include "Core/Memory.h"

#include "Core/DataStructures/Handle.h"

// Internal Math
#include "Core/Math/MathTypes.h"
#include "Core/Math/Math.h"
#include "Core/Math/Vector.h"
