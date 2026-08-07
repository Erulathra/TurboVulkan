#include "Windows/ComponentEditors.h"

#include "Graphics/PostProcess.h"
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

	static TAutoComponentEditor<FLightComponent> LightComponentEditor(
		FName("Light"),
		FDrawComonentPropertyEditorDelegate::CreateLambda([](entt::registry& registry, entt::entity entity)
		{
			FLightComponent& lightComponent = registry.get<FLightComponent>(entity);

			if (ImGui::BeginCombo("Type", ToString(lightComponent.mType)))
			{
				for (uint8 typeId = 0; typeId < static_cast<uint8>(ELightType::Num); ++typeId)
				{
					ELightType currentType = static_cast<ELightType>(typeId);

					ImGui::PushID(static_cast<int32>(typeId));
					if (ImGui::Selectable(ToString(currentType), typeId == static_cast<uint8>(lightComponent.mType)))
					{
						lightComponent.mType = static_cast<ELightType>(typeId);
					}

					ImGui::PopID();
				}

				ImGui::EndCombo();
			}

			ImGui::ColorEdit3("Color", glm::value_ptr(lightComponent.mColor));
			ImGui::DragFloat("Intensity", &lightComponent.mIntensity, 0.1f, 0.f);

			if (lightComponent.mType == ELightType::Point || lightComponent.mType == ELightType::Spot)
			{
				ImGui::DragFloat("Range", &lightComponent.mRange, 0.1f, 0.f);
			}

			if (lightComponent.mType == ELightType::Spot)
			{
				float innerAngleDeg = glm::degrees(lightComponent.mInnerAngle);
				float outerAngleDeg = glm::degrees(lightComponent.mOuterAngle);
				if (ImGui::DragFloat("InnerAngle", &innerAngleDeg, 0.1f, 0.f, 180.f))
				{
					lightComponent.mInnerAngle = glm::min(innerAngleDeg, outerAngleDeg);
				}

				if (ImGui::DragFloat("OuterAngle", &outerAngleDeg, 0.1f, 0.f, 180.f))
				{
					lightComponent.mOuterAngle = glm::max(innerAngleDeg, outerAngleDeg);
				}
			}
		})
	);

	static TAutoComponentEditor<FPostProcessSettings> ToneMapperEditor(
		FName("Post Process"),
		FDrawComonentPropertyEditorDelegate::CreateLambda([](entt::registry& registry, entt::entity entity)
		{
			FPostProcessSettings& settings = registry.get<FPostProcessSettings>(entity);
			ImGui::SeparatorText("Exposure");
			ImGui::DragFloat("EV100", &settings.mEV100, 0.1f, -20.f, 20.f);
			ImGui::SeparatorText("AgX");
			ImGui::DragFloat("Saturation", &settings.mAgXSaturation, 0.1f);
			ImGui::DragFloat3("Offset", glm::value_ptr(settings.mAgXOffset), 0.1f);
			ImGui::DragFloat3("Slope", glm::value_ptr(settings.mAgXSlope), 0.1f);
			ImGui::DragFloat3("Power", glm::value_ptr(settings.mAgXPower), 0.1f);
		})
	);

	static TAutoComponentEditor<FWorldSettings> WorldSettingsEditor(
		FName("World Settings"),
		FDrawComonentPropertyEditorDelegate::CreateLambda([](entt::registry& registry, entt::entity entity)
		{
			FWorldSettings& settings = registry.get<FWorldSettings>(entity);
			ImGui::DragFloat("Ambient Light", &settings.mAmbientLight, 0.1f);
		})
	);

	void PropertyEditors::DummyFunction()
	{
	}
} // Turbo
