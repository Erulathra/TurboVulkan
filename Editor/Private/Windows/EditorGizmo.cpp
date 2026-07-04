#include "Windows/EditorGizmo.h"

#include "EditorLayer.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Core/Engine.h"
#include "Core/Input/Keys.h"
#include "World/Camera.h"
#include "World/World.h"

namespace Turbo
{
	namespace Icons
	{
		const static std::string World = TEXT("");
		const static std::string Local = TEXT("");

		const static std::string Translate = TEXT("󰵉");
		const static std::string Rotate = TEXT("");
		const static std::string Scale = TEXT("󰘖");
	}

	namespace Actions
	{
		const FActionBinding kToggleTransformSpace {FName{"EditorGizmo.ToggleSpace"}, EKeys::Grave, EKeyModifier::LeftShift};
		const FActionBinding kSetTranslateMode {FName{"EditorGizmo.Translate"}, EKeys::W};
		const FActionBinding kSetRotateMode {FName{"EditorGizmo.Rotate"}, EKeys::E};
		const FActionBinding kSetScaleMode {FName{"EditorGizmo.Scale"}, EKeys::R};
	}

	void FEditorGizmo::Init()
	{
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.RegisterBinding(Actions::kToggleTransformSpace);
		inputSystem.RegisterBinding(Actions::kSetTranslateMode);
		inputSystem.RegisterBinding(Actions::kSetRotateMode);
		inputSystem.RegisterBinding(Actions::kSetScaleMode);
	}

	void FEditorGizmo::Draw()
	{
		entt::registry& registry = gEngine->GetWorld()->mRegistry;
		const entt::entity selection = entt::locator<FEditorSelection>::value().GetSelection();

		if (registry.valid(selection) == false || registry.all_of<FTransform>(selection) == false)
		{
			return;
		}

		// Draw gizmo settings
		{
			constexpr int32 IconFontSize = 20;
			constexpr glm::float2 ButtonSize = glm::float2(IconFontSize + 8);
			const glm::uint2 ContentRegionMin = ImGui::GetWindowContentRegionMin();
			ImGui::SetCursorPos(ContentRegionMin + glm::uint2(10.f));

			ImGui::PushFont(nullptr, IconFontSize);

			std::string buttonName = mGizmoSpace == EGizmoSpace::Local ? Icons::Local : Icons::World;
			buttonName += "##GizmoModeButton";
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
			if (ImGui::Button(buttonName.c_str(), ButtonSize))
			{
				mGizmoSpace = mGizmoSpace == EGizmoSpace::Local ? EGizmoSpace::World : EGizmoSpace::Local;
			}
			ImGui::PopStyleColor();

			ImGui::SameLine();
			ImGui::Text("|");

			auto DrawOperationButton = [&](const char* Icon, EGizmoOperation targetOperation)
			{
				const bool bTargetOperationEnabled = mGizmoOperation == targetOperation;

				ImGui::PushStyleColor(ImGuiCol_Button,
					bTargetOperationEnabled ? ImGui::GetColorU32(ImGuiCol_PlotHistogram) : ImGui::GetColorU32(ImGuiCol_PopupBg));
				ImGui::PushStyleColor(ImGuiCol_Text,
					bTargetOperationEnabled ? ImGui::GetColorU32(ImGuiCol_WindowBg) : ImGui::GetColorU32(ImGuiCol_Text));

				if (ImGui::Button(Icon, ButtonSize))
				{
					mGizmoOperation = targetOperation;
				}

				ImGui::PopStyleColor(2);
			};

			ImGui::SameLine();
			DrawOperationButton(Icons::Translate.c_str(), EGizmoOperation::Translate);
			ImGui::SameLine();
			DrawOperationButton(Icons::Rotate.c_str(), EGizmoOperation::Rotate);
			ImGui::SameLine();
			DrawOperationButton(Icons::Scale.c_str(), EGizmoOperation::Scale);

			ImGui::PopFont();
		}

		// Draw gizmo
		{
			ImGuizmo::BeginFrame();
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::AllowAxisFlip(false);
			ImGuizmo::SetDrawlist();

			const glm::float2 contentPos = ImGui::GetWindowContentRegionMin();
			const glm::float2 contentSize = static_cast<glm::float2>(ImGui::GetWindowContentRegionMax()) - contentPos;
			ImGuizmo::SetRect(contentPos.x, contentPos.y, contentSize.x, contentSize.y);

			const entt::entity mainViewportCamera = FCameraUtils::GetMainViewport(registry);
			const FWorldTransform& cameraTransform = registry.get<FWorldTransform>(mainViewportCamera);
			const FCameraCache& cameraCache = registry.get<FCameraCache>(mainViewportCamera);
			FWorldTransform selectionTransform = registry.get<FWorldTransform>(selection);

			const glm::float4x4 viewMatrix = glm::inverse(cameraTransform.mTransform);
			const glm::float4x4 projectionMatrix = cameraCache.mProjectionMatrix;

			if (TransformUtils::IsFrontOf(cameraTransform, selectionTransform))
			{
				glm::float4x4 newTransform = selectionTransform.mTransform;

				ImGuizmo::OPERATION gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
				ImGuizmo::MODE gizmoMode = ImGuizmo::MODE::LOCAL;
				switch (mGizmoOperation)
				{
				case EGizmoOperation::Translate:
					gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
					break;
				case EGizmoOperation::Rotate:
					gizmoOperation = ImGuizmo::OPERATION::ROTATE;
					break;
				case EGizmoOperation::Scale:
					gizmoOperation = ImGuizmo::OPERATION::SCALE;
					break;
				default:
					TURBO_UNINPLEMENTED();
				}

				switch (mGizmoSpace)
				{
					case EGizmoSpace::Local:
						gizmoMode = ImGuizmo::MODE::LOCAL;
						break;
					case EGizmoSpace::World:
						gizmoMode = ImGuizmo::MODE::WORLD;
						break;
					default:
						TURBO_UNINPLEMENTED();
				}

				const bool bTransformDirty = ImGuizmo::Manipulate(
					glm::value_ptr(viewMatrix),
					glm::value_ptr(projectionMatrix),
					gizmoOperation,
					gizmoMode,
					glm::value_ptr(newTransform)
				);

				if (bTransformDirty)
				{
					const glm::float4x4 parentTransform = SceneGraph::GetParentWorldTransform(registry, selection);
					const glm::float4x4& newLocalTransform = glm::inverse(parentTransform) * newTransform;

					const FTransform newTransformComponent = TransformUtils::TransformFromMatrix(newLocalTransform);

					registry.emplace_or_replace<FTransform>(selection, newTransformComponent);
				}
			}
		}
	}

	void FEditorGizmo::HandleEvent(FEventBase& event)
	{
		FEventDispatcher::Dispatch<FActionEvent>(event, [this](FActionEvent& event)
		{
			if (event.mbDown)
			{
				if (event.mName == Actions::kToggleTransformSpace.mName)
				{
					mGizmoSpace = mGizmoSpace == EGizmoSpace::Local ? EGizmoSpace::World : EGizmoSpace::Local;
					event.Handle();
				}
				else if (event.mName == Actions::kSetTranslateMode.mName)
				{
					mGizmoOperation = EGizmoOperation::Translate;
					event.Handle();
				}
				else if (event.mName == Actions::kSetRotateMode.mName)
				{
					mGizmoOperation = EGizmoOperation::Rotate;
					event.Handle();
				}
				else if (event.mName == Actions::kSetScaleMode.mName)
				{
					mGizmoOperation = EGizmoOperation::Scale;
					event.Handle();
				}
			}
		});
	}
} // Turbo
