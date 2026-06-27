#pragma once

#include "glm/glm.hpp"

#define IM_VEC2_CLASS_EXTRA                                                     \
	constexpr ImVec2(const glm::vec2& f) : x(f.x), y(f.y) {}                     \
	constexpr ImVec2(const glm::ivec2& i) : x(i.x), y(i.y) {}                    \
	constexpr ImVec2(const glm::uvec2& i) : x(i.x), y(i.y) {}                    \
	operator glm::vec2() const { return glm::vec2(x,y); }						        \
	operator glm::ivec2() const { return glm::ivec2(x,y); }						     \
	operator glm::uvec2() const { return glm::uvec2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                     \
	constexpr ImVec4(const glm::vec4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}	  \
	operator glm::vec4() const { return glm::vec4(x,y,z,w); }
