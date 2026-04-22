#include "RuntimeTestLayer.h"

#include "FlyMovement.h"
#include "../../cmake-build-debug-test/_deps/imgui-src/imgui.h"
#include "../../Editor/Public/EditorViewPort/EditorFreeCamera.h"
#include "Core/Engine.h"
#include "Extensions/ImGui/ImGuiExtensions.h"
#include "World/Camera.h"
#include "World/World.h"

namespace Turbo
{
	namespace ActionBindings
	{
		const FActionBinding kExit {FName{"Exit"}, EKeys::Escape};
	}

	void FRuntimeTestLayer::Start()
	{
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.RegisterBinding(ActionBindings::kExit);

		FWindow& window = entt::locator<FWindow>::value();
		window.ShowCursor(false);

		FWorld* world = gEngine->GetWorld();
		world->OpenLevel(FName("Content/External/main_sponza/SponzaCompressed.gltf"));

		FFlyMovementSystem::Enable();

		entt::entity player = world->mRegistry.create();
		FCameraUtils::InitializeFreeCamera(world->mRegistry, player);
		FCameraUtils::SetMainViewport(world->mRegistry, player);
	}

	void FRuntimeTestLayer::Shutdown()
	{
	}

	void FRuntimeTestLayer::OnEvent(FEventBase& event)
	{
		ILayer::OnEvent(event);

		FFlyMovementSystem::HandleEvent(event);

		FEventDispatcher::Dispatch<FActionEvent>(
			event, [](FActionEvent& event)
			{
				if (event.mName == ActionBindings::kExit.mName)
				{
					gEngine->RequestExit(EExitCode::Success);
					event.Handle();
				}
			}
		);
	}

	void FRuntimeTestLayer::BeginTick(double deltaTime)
	{
		FFlyMovementSystem::Tick(deltaTime);

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, ImGui::GetTextLineHeightWithSpacing() + (2.f * ImGui::GetStyle().FramePadding.y)));
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags imGuiWindowFlags = 0
			| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::Begin("Toolbar", nullptr, imGuiWindowFlags);
		ImGui::PopStyleVar(1);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 1.f, 0.f, 1.f));
		ImGui::TextFmt("Frame time: {:.3f}ms, FPS: {:.2f}", deltaTime * 1000.f, 1.f / deltaTime);
		ImGui::PopStyleColor();

		ImGui::End();
	}

	bool FRuntimeTestLayer::ShouldTick()
	{
		return true;
	}

	FName FRuntimeTestLayer::GetName()
	{
		const static FName kName("RuntimeTest");
		return kName;
	}
} // Turbo