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

// stl collections
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

// Internal headers
#include "CommonMacros.h"
#include "LoggingMacros.h"
#include "CommonTypeDefs.h"
#include "ProfilingMacros.h"
#include "Core/Name.h"

#include "Core/DataStructures/Pool.h"

// GLM
#include "Core/Math/MathCommon.h"

// Internal Math
#include "Core/Math/Math.h"

#include "magic_enum/magic_enum.hpp"

// VMA
#define VULKAN_HPP_ASSERT_ON_RESULT {}
#include "vk_mem_alloc.hpp"

// EnTT
#include "entt/entt.hpp"
