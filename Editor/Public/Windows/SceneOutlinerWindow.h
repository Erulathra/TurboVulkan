#pragma once

struct ImGuiTextFilter;

namespace Turbo
{
	class FSceneOutlinerWindow
	{
	public:
		void Draw();

		void DrawTree();
		void DrawList(ImGuiTextFilter& filter);

		bool DrawNode(entt::registry& registry, entt::entity entity, bool bForceLeaf = false);
	};
} // Turbo
