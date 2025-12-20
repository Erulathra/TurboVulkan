#include "imgui.h"
#include "RenderingTestLayer.h"
#include "Assets/AssetManager.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Core/Math/Random.h"
#include "Core/Math/Vector.h"
#include "glm/gtx/compatibility.hpp"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "World/MeshComponent.h"
#include "World/World.h"

using namespace Turbo;

struct FMaterialUniforms
{
	glm::float3 mColor;
};

struct FRotateComponent
{
	float speed;
};

entt::entity CreatePivot(FWorld& world, entt::entity parent, float offset, float rotationSpeed)
{
	entt::registry& registry = world.mRegistry;

	// create entity
	const entt::entity entity = registry.create();
	registry.emplace<FRelationship>(entity);
	if (parent != entt::null)
	{
		world.AddChild(parent, entity);
	}

	// initialize components
	FTransform& transform = registry.emplace<FTransform>(entity);
	transform.mPosition.x = offset;
	transform.mRotation = glm::quat(glm::float3(0.f, glm::tau<float>(), 0.f) * Random::RandomFloat());
	FRotateComponent& rotateComponent = registry.emplace<FRotateComponent>(entity);
	rotateComponent.speed = rotationSpeed;

	return entity;
}

entt::entity CreateCelestialBody(FWorld& world, entt::entity parent, float offset, float rotationSpeed, THandle<FMesh> meshHandle, THandle<FMaterial> matHandle)
{
	entt::entity entity = CreatePivot(world, parent, offset, rotationSpeed);

	FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
	THandle<FMaterial::Instance> instanceHandle = materialManager.CreateMaterialInstance(matHandle);
	FMaterial::Instance* matInstance = entt::locator<FMaterialManager>::value().AccessInstance(instanceHandle);
	FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

	gpu.ImmediateSubmit(FOnImmediateSubmit::CreateLambda(
		[&](FCommandBuffer& cmd)
		{
			FMaterialUniforms materialUniforms = {};
			materialUniforms.mColor = Random::RandomColor();

			TURBO_LOG(LogTemp, Info, "PlanetColor: {}", materialUniforms.mColor);

			FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
			materialManager.UpdateMaterialInstance<FMaterialUniforms>(cmd, instanceHandle, &materialUniforms);
		}));

	FMeshComponent& meshComponent = world.mRegistry.emplace<FMeshComponent>(entity);
	meshComponent.mMesh = meshHandle;
	meshComponent.mMaterial = matInstance->material;
	meshComponent.mMaterialInstance = instanceHandle;

    return entity;
}

FRenderingTestLayer::~FRenderingTestLayer()
{
}

FRenderingTestLayer::FRenderingTestLayer()
{
}

void FRenderingTestLayer::Start()
{
	TRACE_ZONE_SCOPED()

	FWorld& world = *gEngine->GetWorld();

	FAssetManager& assetManager = entt::locator<FAssetManager>::value();
	FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();

	std::array meshes = {
		assetManager.LoadMesh("Content/Meshes/SM_BlenderMonkey.glb").front(),
		assetManager.LoadMesh("Content/Meshes/SM_Cube.glb").front(),
		assetManager.LoadMesh("Content/Meshes/SM_IcoPlanet.glb").front(),
	};

	FPipelineBuilder pipelineBuilder = FMaterialManager::CreateOpaquePipeline("BaseMaterial.slang");
	THandle<FMaterial> materialHandle = materialManager.LoadMaterial<FMaterialUniforms>(pipelineBuilder, 256);

	mSunEntity = CreateCelestialBody(world, entt::null, 0.f, 0.f, meshes[0], materialHandle);

	for (uint32 planetId = 1; planetId < 16; ++planetId)
	{
		constexpr float offset = 5.f;
		const entt::entity pivot = CreatePivot(world, entt::null, 0.f, 1.f / static_cast<float>(planetId));
		const entt::entity planet = CreateCelestialBody(world, pivot, offset * static_cast<float>(planetId), 1.f, meshes[planetId % meshes.size()], materialHandle);
	}
}

void FRenderingTestLayer::Shutdown()
{
	FWorld& world = *gEngine->GetWorld();
	entt::registry& registry = world.mRegistry;

	entt::dense_set<THandle<FMesh>> usedMeshes;
	entt::dense_set<THandle<FMaterial>> usedMaterials;

	auto view = registry.view<FMeshComponent>();
	for (entt::entity entity : view)
	{
		const FMeshComponent& meshComponent = view.get<FMeshComponent>(entity);
		usedMeshes.emplace(meshComponent.mMesh);
		usedMaterials.emplace(meshComponent.mMaterial);
	}

	FAssetManager& assetManager = entt::locator<FAssetManager>::value();
	for (THandle<FMesh> mesh : usedMeshes)
	{
		assetManager.UnloadMesh({mesh});
	}

	FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
	for (THandle<FMaterial> material : usedMaterials)
	{
		materialManager.DestroyMaterial(material);
	}
}

void FRenderingTestLayer::ShowImGuiWindow()
{
	FWorld* world = gEngine->GetWorld();
	FTransform transform = world->mRegistry.get<FTransform>(mSunEntity);

	ImGui::Begin("Rendering test");
	ImGui::Text("Frame time: %f, FPS: %f", FCoreTimer::DeltaTime(), 1.f / FCoreTimer::DeltaTime());
	ImGui::DragFloat3("Sun Pos", glm::value_ptr(transform.mPosition));
	ImGui::End();

	world->mRegistry.replace<FTransform>(mSunEntity, transform);
}

void FRenderingTestLayer::BeginTick(double deltaTime)
{
	FWorld& world = *gEngine->GetWorld();
	entt::registry& registry = world.mRegistry;
	auto rotationView = registry.view<FTransform, FRotateComponent>();

	for (const entt::entity entity : rotationView)
	{
		FTransform newTransform = rotationView.get<FTransform>(entity);
		const FRotateComponent& rotateComponent = rotationView.get<FRotateComponent>(entity);

		newTransform.mRotation = glm::quat(EFloat3::Up * rotateComponent.speed * static_cast<float>(deltaTime)) * newTransform.mRotation;

		registry.replace<FTransform>(entity, newTransform);
	}

	ShowImGuiWindow();
}

FName FRenderingTestLayer::GetName()
{
	const static FName kName = FName("RenderingTest");
	return kName;
}
