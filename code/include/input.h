#ifndef _INPUT_H_
#define _INPUT_H_

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class Input
{
public:
	Input(HINSTANCE, HWND, int, int);
	~Input();

	void update();
	void getMouseLocation(int& mouseX, int& mouseY);
	bool isEscapePressed();
	bool isSpacePressed();
	bool isWPressed();
	bool isSPressed();
	bool isAPressed();
	bool isDPressed();
	bool isOPressed();
	bool isPPressed();
	bool isCtrlPressed();

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	unsigned char m_keyboardState[256];
	DIMOUSESTATE m_mouseState;

	int m_screenWidth, m_screenHeight;
	int m_mouseX, m_mouseY;

	void readKeyboard();
	void readMouse();
	void processInput();
};

#endif