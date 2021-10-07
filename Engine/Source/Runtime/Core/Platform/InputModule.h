#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Platform/InputDefs.h"
#include "Runtime/Core/System/Module.h"

namespace Omni
{
	struct MousePos;
	class KeyStateListener;
	using KeyStateCallback = void (*)(KeyCode code, void* userData);

	class InputModule : public Module
	{
	public:
		void Destroy() override;
		void Initialize(const EngineInitArgMap&) override;
		void Finalize() override;
		static InputModule& Get();

		//user
		void GetMousePos(MousePos& state) const;
		void GetKeyStates(u32 count, KeyCode* keys, bool* states) const;
		void RegisterListener(KeyCode key, KeyStateListener* listener);
		void UnRegisterlistener(KeyCode key, KeyStateListener* listener);

		//source
		void UpdateMouse(MousePos& newPos, bool leftButton, bool rightButton);
		void OnKeyEvent(KeyCode key, bool pressed);

		//replay related
		void StartRecording(void* ctx);
		void StopRecording();

		void StartPlayback();
		void StopPlayback();
	};
}

