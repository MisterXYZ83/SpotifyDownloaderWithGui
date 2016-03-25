#pragma once

#include <windows.h>
#include <commctrl.h>
#include "mia_window.h"

#define MYTREEVIEW_DEFAULT_ICONSIZE			32

class MyTreeView : public BaseControl, public BaseListener 
{
protected:	
	//HWND hwnd;
	//HWND parent;
	//HINSTANCE inst;
	int w,h,x0,y0;
	HTREEITEM addItem ( HTREEITEM parent, LPWSTR title, void *extradata, HTREEITEM pos, HBITMAP item );
	HIMAGELIST imgList;
	typedef BOOL (*IsEqualRoutine) ( void *a, void *b );
	int defIconIdx;
public:
	MyTreeView ( HINSTANCE hinst, HWND parent, int x0, int y0, int w, int h );
	HTREEITEM getRootItem();
	HTREEITEM getSelected();
	HTREEITEM addItemAsFirst(HTREEITEM parent, LPWSTR title, void *extradata, HBITMAP item = NULL);
	HTREEITEM addItemAsLast(HTREEITEM parent, LPWSTR title, void *extradata, HBITMAP item = NULL);
	BOOL removeItem(HTREEITEM elem);
	BOOL removeAllItem();
	HTREEITEM getItemWithRoutine(HTREEITEM parent, IsEqualRoutine routine, void *obj); 
	BOOL getItem(HTREEITEM elem, TVITEM *item);
	BOOL getNextItem(HTREEITEM elem, TVITEM *item);
	BOOL setItemIcon(HTREEITEM elem, HBITMAP icon);
	BOOL setItemDefaultIcon(HTREEITEM elem);

	int getItemCount();
	void expandNode(HTREEITEM elem, BOOL child = 0);
	void expandList();
	void show();
	HIMAGELIST setImageList(HBITMAP defIcon = NULL);
	void *getExtraData( HTREEITEM item );
	void selectItem( HTREEITEM item );
	virtual LRESULT executeMessage(UINT idcontrol, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//HWND getHWND();
};

