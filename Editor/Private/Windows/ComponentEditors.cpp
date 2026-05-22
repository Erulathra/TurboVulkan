#include "Windows/ComponentEditors.h"

#include "World/SceneGraph.h"
#include "imgui.h"
#include "World/ShadingComponents.h"

namespace Turbo
{
	static TAutoComponentEditor<FTransform> TransformEditor(
		FName("Transform"),
		FDrawComonentPropertyEditorDelegate::CreateLambda([](entt::registry& registry, entt::entity entity)
		{
			bool bDirty = false;
			FTransform transform = registry.get<FTransform>(entity);

			bDirty |= ImGui::DragFloat3("Position", glm::value_ptr(transform.mPosition), 0.1f);

			glm::float3 eulerAngles = glm::eulerAngles(transform.mRotation);
			if (ImGui::DragFloat3("Rotation", glm::value_ptr(eulerAngles), 0.1f))
			{
				transform.mRotation = glm::quat(eulerAngles);
				bDirty |= true;
			}
			bDirty |= ImGui::DragFloat3("Scale", glm::value_ptr(transform.mScale), 0.1f);

			if (bDirty)
			{
				registry.emplace_or_replace<FTransform>(entity, transform);
			}
		})
	);

	static TAutoComponentEditor<FDirectionalLightComponent> DirectionalLightEditor(
		FName("DirectionalLight"),
		FDrawComonentPropertyEditorDelegate::CreateLambda([](entt::registry& registry, entt::entity entity)
		{
			FDirectionalLightComponent& lightComponent = registry.get<FDirectionalLightComponent>(entity);
			ImGui::ColorEdit3("Color", glm::value_ptr(lightComponent.mColor));
			ImGui::DragFloat("Intensity", &lightComponent.mIntensity, 0.1f, 0.f);
		})
	);

	static TAutoComponentEditor<FPointLightComponent> PointLightEditor(
		FName("PointLight"),
		FDrawComonentPropertyEditorDelegate::CreateLambda([](entt::registry& registry, entt::entity entity)
		{
			FPointLightComponent& lightComponent = registry.get<FPointLightComponent>(entity);
			ImGui::ColorEdit3("Color", glm::value_ptr(lightComponent.mColor));
			ImGui::DragFloat("Intensity", &lightComponent.mIntensity, 0.1f);
			ImGui::DragFloat("Radius", &lightComponent.mIntensity, 0.1f, 0.f);
		})
	);

	static TAutoComponentEditor<FSpotLightComponent> SpotLightEditor(
		FName("SpotLight"),
		FDrawComonentPropertyEditorDelegate::CreateLambda([](entt::registry& registry, entt::entity entity)
		{
			FSpotLightComponent& lightComponent = registry.get<FSpotLightComponent>(entity);
			ImGui::ColorEdit3("Color", glm::value_ptr(lightComponent.mColor));
			ImGui::DragFloat("Intensity", &lightComponent.mIntensity, 0.1f);
			ImGui::DragFloat("Radius", &lightComponent.mIntensity, 0.1f, 0.f);

			float innerAngleDeg = glm::degrees(lightComponent.mInnerAngle);
			float outerAngleDeg = glm::degrees(lightComponent.mOuterAngle);
			ImGui::DragFloat("InnerAngle", &innerAngleDeg, 0.1f, 0.f, 180.f);
			ImGui::DragFloat("InnerAngle", &outerAngleDeg, 0.1f, 0.f, 180.f);

			lightComponent.mInnerAngle = glm::min(innerAngleDeg, outerAngleDeg);
			lightComponent.mOuterAngle = glm::max(innerAngleDeg, outerAngleDeg);
		})
	);

	void PropertyEditors::DummyFunction()
	{
	}
} // Turbo