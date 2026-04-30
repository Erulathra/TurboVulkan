#include "Windows/SceneOutlinerWindow.h"

#include "imgui.h"

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

		ImGui::BeginTable("##EntityList", 1, ImGuiTableFlags_RowBg);

		ImGui::EndTable();

		ImGui::PopItemWidth();
		ImGui::End();
	}
} // Turbo