#include "Graphics/Resources.h"
#include "Graphics/GPUDevice.h"

namespace Turbo {
	void FBufferDestroyer::Destroy(FGPUDevice& GPUDevice)
	{
		GPUDevice.DestroyBufferImmediate(*this);
	}

	void FSamplerDestroyer::Destroy(FGPUDevice& GPUDevice)
	{
		GPUDevice.DestroySamplerImmediate(*this);
	}

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

	void FDescriptorPoolDestroyer::Destroy(FGPUDevice& GPUDevice)
	{
		GPUDevice.DestroyDescriptorPoolImmediate(*this);
	}

	void FPipelineDestroyer::Destroy(FGPUDevice& GPUDevice)
	{
		GPUDevice.DestroyPipelineImmediate(*this);
	}
} // Turbo