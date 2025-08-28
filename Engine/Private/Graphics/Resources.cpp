#include "Graphics/Resources.h"
#include "Graphics/GPUDevice.h"

namespace Turbo {
	void FTextureDestroyer::Destroy(FGPUDevice& GPUDevice)
	{
		GPUDevice.DestroyTextureImmediate(*this);
	}
} // Turbo