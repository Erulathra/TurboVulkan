#pragma once

namespace Turbo
{
	class FGPUDevice;
	class FGeometryBuffer;

	struct FGraphicsLocator
	{
	public:
		static void InitGeometryBuffer(FGPUDevice* gpu);

	public:
		static FGeometryBuffer& GetGeometryBuffer();
	};
} // Turbo
