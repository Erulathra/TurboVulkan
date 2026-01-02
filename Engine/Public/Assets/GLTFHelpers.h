#pragma once

#include "fastgltf/core.hpp"

namespace Turbo
{
	class FTurboGLTFDataBuffer final : public fastgltf::GltfDataGetter
	{
	public:
		static FTurboGLTFDataBuffer Load(FName path);

		virtual void read(void* ptr, std::size_t count) override;
		[[nodiscard]] virtual fastgltf::span<byte> read(std::size_t count, std::size_t padding) override;

		virtual void reset() override;

		[[nodiscard]] virtual std::size_t bytesRead() override { return readBytesNum; }
		[[nodiscard]] virtual std::size_t totalSize() override { return mBytes.size(); }

	private:
		std::vector<byte> mBytes;
		size_t readBytesNum = 0;
	};
}
