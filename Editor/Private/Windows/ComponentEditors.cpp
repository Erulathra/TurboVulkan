#include "Windows/ComponentEditors.h"

#include "World/SceneGraph.h"
#include "imgui.h"

namespace Turbo
{
	static TAutoComponentEditor<FTransform> TransformEditor(
		FName("Transform"),
		FDrawComonentPropertyEditorDelegate::CreateLambda([](entt::registry& registry, entt::entity entity)
		{
			bool bDirty = false;
			FTransform transform = registry.get<FTransform>(entity);

			bDirty |= ImGui::DragFloat3("Position", &transform.mPosition.x, 0.1f);

			glm::float3 eulerAngles = glm::eulerAngles(transform.mRotation);
			if (ImGui::DragFloat3("Rotation", &eulerAngles.x, 0.1f))
			{
				transform.mRotation = glm::quat(eulerAngles);
				bDirty |= true;
			}
			bDirty |= ImGui::DragFloat3("Scale", &transform.mScale.x, 0.1f);

			if (bDirty)
			{
				registry.emplace_or_replace<FTransform>(entity, transform);
			}
		})
	);

	void PropertyEditors::DummyFunction()
	{
	}
} // Turbo