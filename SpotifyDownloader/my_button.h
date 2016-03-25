#pragma once

#include <windows.h>
#include <commctrl.h>
#include "mia_window.h"

class MyButton : public BaseControl
{
	
public:

	MyButton ( HINSTANCE hinst, HWND parent, int x0, int y0, int w, int h );
	//virtual LRESULT executeMessage(UINT idcontrol, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//HWND getHWND();
	//virtual BOOL setCaption(char *caption);
	//virtual int getCaption(char *caption, int maxLen);
};

