

#include "my_progressbar.h"
#include <tchar.h>

MyProgressBar::MyProgressBar( HINSTANCE hinst, HWND parent, int x0, int y0, int w, int h )
:BaseControl(hinst, x0, y0, w, h, parent, TEXT("ProgressBar"), PROGRESS_CLASS,PBS_SMOOTH )
{
	//this->inst = hinst;
	//this->parent = parent;
	//this->x0 = x0;
	//this->y0 = y0;
	//this->w = w;
	//this->h = h;
	HFONT hFont;
 
	hFont = CreateFont(14, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));

	HFONT oldFont = (HFONT)SelectObject(GetDC(hWnd), hFont);

	SIZE sz = {0};
	BOOL ret = GetTextExtentPoint32(GetDC(hWnd), TEXT("###########"), _tcslen(TEXT("###########")), &sz);
	
	if ( ret )
	{
		labelHwnd = CreateWindow(TEXT("STATIC"), TEXT("0%"), WS_VISIBLE | WS_CHILD | SS_CENTER, x0 + w + 10, y0 + h/2 - sz.cy/2, sz.cx, sz.cy, parent, NULL, hInstance, NULL);
	}
	else
		labelHwnd = CreateWindow(TEXT("STATIC"), TEXT("0%"), WS_VISIBLE | WS_CHILD | SS_CENTER, x0 + w + 10, y0, 30, h, parent, NULL, hInstance, NULL);

	EnableWindow(labelHwnd, TRUE);

	SelectObject(GetDC(hWnd), oldFont);
	SendMessage(labelHwnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	//DeleteObject(hFont);
}

BOOL MyProgressBar::setRange( int min, int max )
{
	return SendMessage(this->getHWND(),PBM_SETRANGE32,min,max);
}

BOOL MyProgressBar::setPosition ( int pos, TCHAR *title )
{
	TCHAR text [101];
	memset(text, 0, 101);
	int min, max;
	getRange(&min,&max);
	if ( title )
		_sntprintf_s(text, 100, TEXT("%s - %d%%"), title, (pos >= max)?max:pos);
	else
		_sntprintf_s(text, 100, TEXT("%d%%"), (pos >= max)?max:pos);

	SendMessage(labelHwnd, WM_SETTEXT, NULL, (LPARAM)text);

	return SendMessage(this->getHWND(), PBM_SETPOS, pos, 0);
}

BOOL MyProgressBar::getRange( int *min, int *max )
{
	if ( !min || !max ) return FALSE;
	PBRANGE range;
	BOOL res = SendMessage(this->getHWND(),PBM_GETRANGE,0,(LPARAM)&range);
	*min = range.iLow;
	*max = range.iHigh;
	return res;
}

BOOL MyProgressBar::getPosition( int *pos )
{
	if ( pos ){
		*pos = SendMessage(this->getHWND(),PBM_GETPOS,0,0);
		return TRUE;
	} else return FALSE;
}