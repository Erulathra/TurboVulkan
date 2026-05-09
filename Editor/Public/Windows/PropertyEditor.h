#pragma once
#include "Core/Delegate.h"

namespace Turbo
{
	DECLARE_DELEGATE(FDrawComonentPropertyEditorDelegate, entt::registry&, entt::entity);

	struct FComponentEditorRegistration
	{
		entt::id_type mComponentTypeId;
		FName mComponentName;
		FDrawComonentPropertyEditorDelegate mOnDraw;

		FComponentEditorRegistration(entt::id_type componentTypeId, const FName& name, FDrawComonentPropertyEditorDelegate&& onDraw)
			: mComponentTypeId(componentTypeId)
			, mComponentName(name)
			, mOnDraw(std::move(onDraw)) {}
	};

	class FPropertyEditorSystem
	{
	public:
		static FPropertyEditorSystem* Get();

		void RegisterComponentEditor(const FComponentEditorRegistration& componentEditor);
		[[nodiscard]] const std::vector<entt::id_type>& GetRegisteredEditors() const { return mRegisteredEditors; }
		[[nodiscard]] const FComponentEditorRegistration& GetEditor(entt::id_type componentType) const { return mRegisteredComponentEditors.at(componentType); }

	private:
		static FPropertyEditorSystem* instance;

	private:
		entt::dense_map<entt::id_type, FComponentEditorRegistration> mRegisteredComponentEditors;
		std::vector<entt::id_type> mRegisteredEditors;
	};

	template <typename ComponentType>
	struct TAutoComponentEditor
	{
		TAutoComponentEditor(const FName& name, FDrawComonentPropertyEditorDelegate&& onDraw)
		{
			const entt::type_info type = entt::type_id<ComponentType>();
			FPropertyEditorSystem::Get()->RegisterComponentEditor({type.hash(), name, std::move(onDraw)});
		}
	};


	class FPropertyEditorWindow
	{
	public:
		void Init();
		void Draw();
	};
} // Turbo
