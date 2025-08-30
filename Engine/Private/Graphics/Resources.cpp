#include "Graphics/Resources.h"
#include "Graphics/GPUDevice.h"

namespace Turbo {
	void FTextureDestroyer::Destroy(FGPUDevice& GPUDevice)
	{
		GPUDevice.DestroyTextureImmediate(*this);
	}

	void FShaderStateDestroyer::Destroy(FGPUDevice& GPUDevice)
	{
		GPUDevice.DestroyShaderStateImmediate(*this);
	}

	void FDescriptorSetLayoutDestroyer::Destroy(FGPUDevice& GPUDevice)
	{
		GPUDevice.DestroyDescriptorSetLayoutImmediate(*this);
	}
} // Turbo