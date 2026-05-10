#include "Windows/EditorGizmo.h"

#include "EditorLayer.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Core/Engine.h"
#include "Layers/ImGUILayer.h"
#include "World/Camera.h"
#include "World/World.h"

namespace Turbo
{
	void FEditorGizmo::Draw()
	{
		entt::registry& registry = gEngine->GetWorld()->mRegistry;
		const entt::entity selection = entt::locator<FEditorSelection>::value().GetSelection();

		if (registry.valid(selection) && registry.all_of<FTransform>(selection))
		{
			ImGuizmo::BeginFrame();
			ImGuizmo::SetOrthographic(false);
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

			glm::float4x4 newTransform = selectionTransform.mTransform;

			const bool bTransformDirty = ImGuizmo::Manipulate(
				glm::value_ptr(viewMatrix),
				glm::value_ptr(projectionMatrix),
				ImGuizmo::OPERATION::TRANSLATE,
				ImGuizmo::LOCAL,
				glm::value_ptr(newTransform)
			);

			if (bTransformDirty)
			{
				const glm::float4x4 parentTransform = SceneGraph::GetParentWorldTransform(registry, selection);
				const glm::float4x4& newLocalTransform = glm::inverse(parentTransform) * newTransform;

				const FTransform newTransformComponent = FMath::TransformFromMatrix(newLocalTransform);

				registry.emplace_or_replace<FTransform>(selection, newTransformComponent);
			}
		}
	}
} // Turbo