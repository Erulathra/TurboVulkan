#include "Windows/PropertyEditor.h"

#include "EditorLayer.h"
#include "imgui.h"
#include "Core/Engine.h"
#include "Extensions/ImGui/ImGuiExtensions.h"
#include "Windows/ComponentEditors.h"
#include "World/EntityUtils.h"
#include "World/World.h"

namespace Turbo
{
	FPropertyEditorSystem* FPropertyEditorSystem::instance = nullptr;

	FPropertyEditorSystem* FPropertyEditorSystem::Get()
	{
		if (instance == nullptr)
		{
			instance = new FPropertyEditorSystem();
		}

		return instance;
	}

	void FPropertyEditorSystem::RegisterComponentEditor(const FComponentEditorRegistration& componentEditor)
	{
		TURBO_CHECK(mRegisteredComponentEditors.find(componentEditor.mComponentTypeId) == mRegisteredComponentEditors.end())
		mRegisteredComponentEditors.emplace(componentEditor.mComponentTypeId, componentEditor);
		mRegisteredEditors.push_back(componentEditor.mComponentTypeId);
	}

	void FPropertyEditorWindow::Init()
	{
		PropertyEditors::DummyFunction();
	}

	void FPropertyEditorWindow::Draw()
	{
		entt::registry& registry = gEngine->GetWorld()->mRegistry;
		const entt::entity selection = entt::locator<FEditorSelection>::value().GetSelection();
		const FPropertyEditorSystem* propertyEditorSystem = FPropertyEditorSystem::Get();

		ImGui::Begin("Property Editor");

		ImGui::Separator();
		ImGui::TextFmt("PropertyEditor: {}", EntityUtils::GetEntityLabel(registry, selection));
		ImGui::Separator();

		if (registry.valid(selection))
		{
			const std::vector<entt::id_type>& registeredEditors = propertyEditorSystem->GetRegisteredEditors();
			for (entt::id_type typeHash : registeredEditors)
			{
				const auto* componentStorage = registry.storage(typeHash);
				if (componentStorage && componentStorage->contains(selection))
				{
					const FComponentEditorRegistration& editor = propertyEditorSystem->GetEditor(typeHash);

					ImGui::PushID(typeHash);

					if (ImGui::CollapsingHeader(editor.mComponentName.ToCString()))
					{
						editor.mOnDraw.Execute(registry, selection);
					}

					ImGui::PopID();
				}
			}
		}

		ImGui::End();
	}
} // Turbo
