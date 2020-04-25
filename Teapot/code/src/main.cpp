#include "../include/window.h"
#include "../include/constant_parameters.h"
#include <stdio.h>



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	Window* window = new Window();
	
	if (window->init())
		window->run();

	delete window;
	window = 0;

	return 0;
}