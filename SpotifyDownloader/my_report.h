#pragma once

#include <windows.h>
#include <commctrl.h>
#include "mia_window.h"

class MyReport : public BaseControl
{

public:
	MyReport ( HINSTANCE hinst, HWND parent, int x0, int y0, int w, int h );
	//virtual LRESULT executeMessage( UINT msg, WPARAM wParam, LPARAM lParam );
	virtual BOOL setCaption(TCHAR *caption);
	virtual int getCaption(TCHAR *caption, int maxLen);

	BOOL addRow( TCHAR *format, ... );
	BOOL removeAllRows ( );
};

