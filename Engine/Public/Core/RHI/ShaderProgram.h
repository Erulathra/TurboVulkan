#pragma once

#include "RHICore.h"

namespace Turbo {
	class Device;

	class ShaderProgram {

public:
	ShaderProgram() = default;
	virtual ~ShaderProgram();

public:
	void Init(const std::vector<uint8>& ShaderCode, const Device* InDevice);

private:
	VkShaderModule ShaderModule = nullptr;
};

} // Turbo
