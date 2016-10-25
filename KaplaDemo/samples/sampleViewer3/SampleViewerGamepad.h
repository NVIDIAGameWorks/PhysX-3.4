#ifndef SAMPLE_VIEWER_GAMEPAD_H
#define SAMPLE_VIEWER_GAMEPAD_H

#include <WTypes.h>
#include <XInput.h>

class SampleViewerScene;

class SampleViewerGamepad
{
public:
	enum GamepadTrigger
	{
		GAMEPAD_RIGHT_SHOULDER_TRIGGER = 0,
		GAMEPAD_LEFT_SHOULDER_TRIGGER = 1
	};

	enum GamepadButtons
	{
		GAMEPAD_DPAD_UP = 0,
		GAMEPAD_DPAD_DOWN,
		GAMEPAD_DPAD_LEFT,
		GAMEPAD_DPAD_RIGHT,
		GAMEPAD_START,
		GAMEPAD_BACK,
		GAMEPAD_LEFT_THUMB,
		GAMEPAD_RIGHT_THUMB,
		GAMEPAD_Y,
		GAMEPAD_A,
		GAMEPAD_X,
		GAMEPAD_B,
		GAMEPAD_LEFT_SHOULDER,
		GAMEPAD_RIGHT_SHOULDER
	};

	enum GamepadAxis
	{
		GAMEPAD_RIGHT_STICK_X = 0,
		GAMEPAD_RIGHT_STICK_Y,
		GAMEPAD_LEFT_STICK_X,
		GAMEPAD_LEFT_STICK_Y
	};

	SampleViewerGamepad();
	~SampleViewerGamepad();

	void							init();
	void							release();

	void							processGamepads(SampleViewerScene& viewerScene);

private:
	bool							hasXInput() const { return mpXInputGetState && mpXInputGetCapabilities; }

private:

	bool							mGamePadConnected;
	unsigned int					mConnectedPad;

	typedef DWORD(WINAPI *LPXINPUTGETSTATE)(DWORD, XINPUT_STATE*);
	typedef DWORD(WINAPI *LPXINPUTGETCAPABILITIES)(DWORD, DWORD, XINPUT_CAPABILITIES*);

	HMODULE							mXInputLibrary;
	LPXINPUTGETSTATE				mpXInputGetState;
	LPXINPUTGETCAPABILITIES			mpXInputGetCapabilities;

};

#endif // SAMPLE_VIEWER_GAMEPAD_H
