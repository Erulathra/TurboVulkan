#pragma once

namespace Turbo
{
	class FEditorGizmo;
	struct FTexture;
	struct FEventBase;

	class FEditorViewportWindow
	{
	public:
		void Init();
		void Shutdown();

		void HandleEvent(FEventBase& event);

		void Tick(float deltaTime);
		void Draw();
		void ResizeViewport(const glm::uint2& newSize);

	public:
		FEditorViewportWindow();
		~FEditorViewportWindow();

	public:
		TUniquePtr<FEditorGizmo> mGizmo;

		std::vector<THandle<FTexture>> mRenderedTextures;
		glm::uint2 mEditorViewportSize = glm::uint2(0);

		bool bHasFocus = false;
	};
} // Turbo
