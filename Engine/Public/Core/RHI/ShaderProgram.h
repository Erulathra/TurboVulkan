#pragma once

#include "RHICore.h"

namespace Turbo {

class ShaderProgram {

public:
	ShaderProgram() = default;
	virtual ~ShaderProgram();

public:
	void Init(const std::vector<uint8>& ShaderCode, const LogicalDevicePtr& Device);

private:
	LogicalDeviceWeakPtr WeakDevice;
	VkShaderModule ShaderModule = nullptr;
};

} // Turbo
