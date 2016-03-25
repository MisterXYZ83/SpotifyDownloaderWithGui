#pragma once

#include <Windows.h>
#include "mia_window.h"


class BaseMenuCommandListener 
{
public:
	virtual LRESULT dispatchCommand(HMENU menu, UINT mess, WPARAM wParam, LPARAM lParam ) = 0;
};


class BasePopupMenu : public BaseWindow 
{

protected:
	HMENU menu;
	MENUINFO menuInfo;
	int lastItemPos;
	virtual LRESULT CALLBACK wndProc(UINT, WPARAM, LPARAM);
	BaseMenuCommandListener *listener;
public:
	BasePopupMenu();
	void showMenu(int x, int y, HWND owner);
	BOOL insertTextItem(int idx, LPWSTR title, void *extraData);
	BOOL insertSeparator();
	BOOL insertBitmapItem(int idx, LPWSTR title, HBITMAP img,  void *extraData);
	void setBaseCommandListener(BaseMenuCommandListener *l);
	static BasePopupMenu *getMenu( HMENU );
};