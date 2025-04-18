#include "../include/input.h"


Input::Input(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	m_directInput = 0;
	m_keyboard = 0;
	m_mouse = 0;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_mouseX = 0;
	m_mouseY = 0;

	DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	m_keyboard->Acquire();
	m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	m_mouse->SetDataFormat(&c_dfDIMouse);
	m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	m_mouse->Acquire();
}

Input::~Input()
{
	m_mouse->Unacquire();
	m_mouse->Release();
	m_mouse = 0;
	m_keyboard->Unacquire();
	m_keyboard->Release();
	m_keyboard = 0;
	m_directInput->Release();
	m_directInput = 0;
}

void Input::update()
{
	readKeyboard();
	readMouse();
	processInput();
}

void Input::readKeyboard()
{
	m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	m_keyboard->Acquire();
}

void Input::readMouse()
{
	m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	m_mouse->Acquire();
}

void Input::processInput()
{
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;
}

void Input::getMouseLocation(int& mouseX, int& mouseY)
{
	mouseX = m_mouseX;
	mouseY = m_mouseY;
}

bool Input::isEscapePressed()
{
	if (m_keyboardState[DIK_ESCAPE] & 0x80) return true;
	else return false;
}

bool Input::isSpacePressed()
{
	if (m_keyboardState[DIK_SPACE] & 0x80) return true;
	else return false;
}

bool Input::isWPressed()
{
	if (m_keyboardState[DIK_W] & 0x80) return true;
	else return false;
}

bool Input::isSPressed()
{
	if (m_keyboardState[DIK_S] & 0x80) return true;
	else return false;
}

bool Input::isAPressed()
{
	if (m_keyboardState[DIK_A] & 0x80) return true;
	else return false;
}

bool Input::isDPressed()
{
	if (m_keyboardState[DIK_D] & 0x80) return true;
	else return false;
}

bool Input::isOPressed()
{
	if (m_keyboardState[DIK_O] & 0x80) return true;
	else return false;
}

bool Input::isPPressed()
{
	if (m_keyboardState[DIK_P] & 0x80) return true;
	else return false;
}

bool Input::isCtrlPressed()
{
	if (m_keyboardState[DIK_LCONTROL] & 0x80) return true;
	else return false;
}