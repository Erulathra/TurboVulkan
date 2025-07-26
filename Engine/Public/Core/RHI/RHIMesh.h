#pragma once

#include "Buffer.h"
#include "VulkanRHI.h"
#include "Core/Engine.h"
#include "Core/RHI/RHICore.h"

namespace Turbo
{
	struct FSubMesh
	{
		uint32 StartVertexIndex;
		uint32 NumIndices;
	};

	struct FRHIMeshCreationInfo
	{
		std::vector<glm::vec3> Positions{};
		std::vector<glm::vec3> Normals{};
		std::vector<glm::vec2> UVs{};
		std::vector<glm::vec4> Colors{};

		std::vector<uint32> Indices{};

		std::vector<FSubMesh> SubMeshes{};

	public:
		[[nodiscard]] bool IsValid() const;
		[[nodiscard]] size_t GetVertexBufferSize() const;

		void Reserve(size_t numVertices, size_t numIndices);
	};

	enum class EVertexComponents : uint8
	{
		None = 0,

		Position = 1 << 0,
		Normal = 1 << 1,
		UV = 1 << 2,
		Color = 1 << 3,

		Basic = Position | Normal | UV,
		All = Position | Normal | UV | Color
	};
	DEFINE_ENUM_OPERATORS(EVertexComponents)

	class FRHIMesh
	{
		GENERATED_BODY(FRHIMesh)

	private:
		FRHIMesh() = default;

	public:
		static std::shared_ptr<FRHIMesh> CreateShared(FVulkanDevice* device, const FRHIMeshCreationInfo& creationInfo);
		static std::unique_ptr<FRHIMesh> CreateUnique(FVulkanDevice* device, const FRHIMeshCreationInfo& creationInfo);

	public:
		void Draw(vk::CommandBuffer cmd, uint32 instanceCount = 1);

	public:
		[[nodiscard]] std::shared_ptr<FBuffer> GetVertexBuffer() const { return mVertexBuffer; }
		[[nodiscard]] std::shared_ptr<FBuffer> GetIndexBuffer() const { return mIndexBuffer; }

		[[nodiscard]] vk::DeviceAddress GetPositionsAddress() const { return mPositionsAddress; }
		[[nodiscard]] vk::DeviceAddress GetNormalsAddress() const { return mNormalsAddress; }
		[[nodiscard]] vk::DeviceAddress GetUvAddress() const { return mUvAddress; }
		[[nodiscard]] vk::DeviceAddress GetColorAddress() const { return mColorAddress; }

	private:
		void Init(FVulkanDevice* device, const FRHIMeshCreationInfo& CreationInfo);

	private:
		std::shared_ptr<FBuffer> mVertexBuffer = nullptr;
		std::shared_ptr<FBuffer> mIndexBuffer = nullptr;

		vk::DeviceAddress mPositionsAddress = 0;
		vk::DeviceAddress mNormalsAddress = 0;
		vk::DeviceAddress mUvAddress = 0;
		vk::DeviceAddress mColorAddress = 0;

		EVertexComponents mVertexComponents = EVertexComponents::None;

		std::vector<FSubMesh> mSubMeshes{};
	};

} // Turbo
