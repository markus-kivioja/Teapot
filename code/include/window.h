#ifndef _WINDOWX_H_
#define _WINDOWX_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

class Input;
class Scene;

class Window
{
public:
	Window();
	~Window();

	bool init();

	void run();

	LRESULT CALLBACK messageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

private:
	int m_lastTime;

	Scene* m_scene;
	Input* m_input;

	LPCSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	void update();
};

static LRESULT CALLBACK s_windowProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
static Window* s_window = 0;

#endif