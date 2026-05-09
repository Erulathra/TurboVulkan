#include "Windows/SceneOutlinerWindow.h"

#include <queue>

#include "EditorLayer.h"
#include "imgui.h"
#include "Core/Engine.h"
#include "Extensions/ImGui/ImGuiExtensions.h"
#include "Layers/Layer.h"
#include "World/EntityUtils.h"
#include "World/World.h"

namespace Turbo
{
	void FSceneOutlinerWindow::Draw()
	{
		ImGui::Begin("Scene Outliner");

		static ImGuiTextFilter textFilter;
		ImGui::PushItemWidth(-1);
		if (ImGui::InputTextWithHint("##Filter", "Filter", textFilter.InputBuf, 256, ImGuiInputTextFlags_EscapeClearsAll))
		{
			textFilter.Build();
		}

		if (textFilter.IsActive())
		{
			DrawList(textFilter);
		}
		else
		{
			DrawTree();
		}

		ImGui::PopItemWidth();
		ImGui::End();
	}

	void FSceneOutlinerWindow::DrawTree()
	{
		entt::registry& registry = gEngine->GetWorld()->mRegistry;

		ImGui::BeginTable("##EntityList", 1, ImGuiTableFlags_RowBg);

		// breath-first search
		std::vector<std::pair<entt::entity, uint32>> entitiesToProcess;

		auto rootView = registry.view<FWorldRoot>();
		for (entt::entity entity : rootView)
		{
			entitiesToProcess.emplace_back(entity, 0);
		}

		uint32 currentDepth = 0;

		while (entitiesToProcess.empty() == false)
		{
			auto [currentEntity, nodeDepth] = entitiesToProcess.back();
			entitiesToProcess.pop_back();

			ImGui::TreePop(currentDepth, nodeDepth);

			if (DrawNode(registry, currentEntity))
			{
				currentDepth++;
				FSceneGraph::EachChild(
					registry, currentEntity, [&](entt::entity child)
				{
					entitiesToProcess.emplace_back(child, nodeDepth + 1);
				});
			}
		}

		ImGui::TreePop(currentDepth, 0);
		ImGui::EndTable();
	}

	bool FSceneOutlinerWindow::DrawNode(entt::registry& registry, entt::entity entity, bool bForceLeaf)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		const std::string label = EntityUtils::GetEntityLabel(registry, entity);
		const FRelationship& relationship = registry.get<FRelationship>(entity);

		ImGuiTreeNodeFlags nodeFlags = 0;
		nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
		nodeFlags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent;
		nodeFlags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DrawLinesToNodes;

		if (entity == entt::locator<FEditorSelection>::value().GetSelection())
		{
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
		}

		if (relationship.mNumChildren == 0 || bForceLeaf)
		{
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		ImGui::SetNextItemStorageID(static_cast<uint32>(entity));
		bool bOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity)), nodeFlags, "%s", label.c_str());
		bOpen &= relationship.mNumChildren > 0;

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("Entity", &entity, sizeof(entt::entity));
			ImGui::TextFmt("{}", label.c_str());

			ImGui::EndDragDropSource();
		}

		if (ImGui::IsItemClicked())
		{
			entt::locator<FEditorSelection>::value().SetSelection(entity);
		}

		return bOpen;
	}

	void FSceneOutlinerWindow::DrawList(ImGuiTextFilter& filter)
	{
		entt::registry& registry = gEngine->GetWorld()->mRegistry;

		ImGui::BeginTable("##EntityList", 1, ImGuiTableFlags_RowBg);

		auto namedEntitiesView = registry.view<FEntityLabel>();
		for (entt::entity entity : namedEntitiesView)
		{
			const FEntityLabel& entityName = namedEntitiesView.get<FEntityLabel>(entity);
			if (filter.PassFilter(entityName.mName.ToCString()))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				DrawNode(registry, entity, true);
			}
		}

		ImGui::EndTable();
	}
} // Turbo