#include "../include/window.h"
#include "../include/input.h"
#include "../include/scene.h"
#include "../include/constant_parameters.h"
#include <sys/timeb.h>

Window::Window() :
	m_lastTime(0)
{
}

Window::~Window()
{
	delete m_scene;
	delete m_input;
	ShowCursor(true);
	ChangeDisplaySettings(0, 0);
	DestroyWindow(m_hwnd);
	UnregisterClass(m_applicationName, m_hinstance);
	m_scene = 0;
	m_input = 0;
	m_hwnd = 0;
	m_hinstance = 0;
	s_window = 0;
}

bool Window::init()
{
	int screenWidth = 0;
	int screenHeight = 0;

	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	s_window = this;

	m_hinstance = GetModuleHandle(NULL);

	m_applicationName = "Teapot";

	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = s_windowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if(FULLSCREEN)
	{
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = posY = 0;
	}
	else
	{
		screenWidth  = 800;
		screenHeight = 600;
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}


	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, 
						    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
						    posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	ShowCursor(false);

	m_input = new Input(m_hinstance, m_hwnd, screenWidth, screenHeight);

	m_scene = new Scene(m_input);

	return m_scene->init(screenWidth, screenHeight, m_hwnd);
}

void Window::run()
{
	while(1)
	{
		update();
		if(m_input->isEscapePressed())
			break;
	}
}


void Window::update()
{
	SYSTEMTIME time;
	GetSystemTime(&time);

	int curTime = time.wMilliseconds;

	int dt = curTime - m_lastTime;
	if (dt < 0)
		dt += 1000;
	m_lastTime = curTime;
	
	m_input->update();

	m_scene->update((int)(dt*RUN_SPEED_FACTOR));
}

LRESULT CALLBACK Window::messageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

LRESULT CALLBACK s_windowProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);		
			return 0;
		}
		default:
		{
			return s_window->messageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}