#include "Graphics/GraphicsLocator.h"

#include "Graphics/GeometryBuffer.h"

namespace Turbo
{
	FGeometryBuffer* gGeometryBuffer = nullptr;

	void FGraphicsLocator::InitGeometryBuffer(FGPUDevice* gpu)
	{
		gGeometryBuffer = new FGeometryBuffer(gpu);
	}

	FGeometryBuffer& FGraphicsLocator::GetGeometryBuffer()
	{
		TURBO_CHECK(gGeometryBuffer);

		return *gGeometryBuffer;
	}
} // Turbo