#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/Module.h"


namespace Omni
{
	
	using KeyCode = u16;

	struct CursorState
	{//origin is at bottom left, x go right, y go up
		u16 X;
		u16 Y;
	};
	struct KeyStateListener
	{
		virtual void OnKeyEvent(KeyCode key, bool pressed) = 0;
	};
	using KeyStateCallback = void (*)(KeyCode code, void* userData);

	constexpr KeyCode OmniKeyLButton = (KeyCode )-1;
	constexpr KeyCode OmniKeyRButton = (KeyCode)-2;


	class InputModule : public Module
	{
	public:
		void Initialize(const EngineInitArgMap&) override;
		void Finalize() override;
		void Finalizing() override;
		static InputModule& Get();

		//user
		void GetCursorState(CursorState& state);
		void GetKeyStates(u32 count, KeyCode* keys, bool* states);
		void RegisterListener(KeyCode key, KeyStateListener* listener);
		void UnRegisterlistener(KeyCode key, KeyStateListener* listener);

		//source
		void UpdateCursorState(CursorState& newState);
		void OnKeyEvent(KeyCode key, bool pressed);

		//replay related
		void StartRecording(void* ctx);
		void StopRecording();

		void StartPlayback();
		void StopPlayback();
	};
}

