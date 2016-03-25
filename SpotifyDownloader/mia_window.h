#pragma once

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <Shlwapi.h>

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib, "Shlwapi.lib")

using namespace Gdiplus;

#define MY_WINDOW_CLASSNAME "MyWindow"
#define DEFAULT_CAPTION "DefaultCaption"
#define DEFAULT_X0 100
#define DEFAULT_Y0 100
#define DEFAULT_LX 800
#define DEFAULT_LY 600

class PaintRoutine
{
public:
	virtual void onPaint(HDC hdc, PAINTSTRUCT ps, void *extra) = 0;
};

class CloseListener
{
public:
	virtual int onClose(int idWin, HWND hwnd) = 0;
};

class BaseListener
{
public:
	virtual LRESULT executeMessage(UINT idcontrol, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
};

class BaseWindow
{
protected:
	HINSTANCE hInstance;
	HWND hWnd;
	HWND parentHwnd;
	WNDPROC oldProc;
	UINT idWin;
	virtual LRESULT CALLBACK wndProc(UINT, WPARAM, LPARAM) = 0;
	CloseListener *closeList;
	BaseListener *listener;
public:
	int getID();
	BOOL setID(UINT id);

	static BaseWindow *getInstance(HWND hwnd);
	~BaseWindow();

	static LRESULT CALLBACK baseWndProc(HWND, UINT, WPARAM, LPARAM);
	HWND getHWND();
	HWND getParentHWND();
	WNDPROC getOldProc();
	void setBaseListener( BaseListener *listener );
	virtual BOOL setCaption(TCHAR *caption);
	virtual int getCaption(TCHAR *caption, int maxLen);
	BOOL setEnableState(BOOL val);
	BOOL setOnCloseListener( CloseListener *list );
};

class MyWindow : public BaseWindow
{
protected:
	//HWND hWnd;
	WNDCLASS wndClass;
	BOOL isMainFlag;

	virtual LRESULT CALLBACK wndProc(UINT, WPARAM, LPARAM);

	//static MyWindow *getInstance(HWND hwnd);

	PaintRoutine *graphRoutine;
public:
	//classe della window
	static const TCHAR WindowClass[];
	//costruttori
	MyWindow(HINSTANCE h, int x0 = DEFAULT_X0, int y0 = DEFAULT_Y0,
						  int lx = DEFAULT_LX, int ly = DEFAULT_LY,
						  UINT wStyle = WS_OVERLAPPEDWINDOW,
						  UINT style = CS_HREDRAW | CS_VREDRAW,
						  HBRUSH back = (HBRUSH)GetStockObject(LTGRAY_BRUSH),
						  HWND parent = NULL,
						  BOOL isMain = FALSE);
	//Default Window procedure
	//static LRESULT CALLBACK baseWndProc(HWND, UINT, WPARAM, LPARAM);
	void show(int showMode = SW_SHOWNORMAL);
	//HWND getHWND();
	BOOL isMain();
	void setAsMainWindow(BOOL);

	virtual void onDestroy();
	virtual void setPaintCallback(PaintRoutine *r);

};

#define INSTANCE_PROP TEXT("INSTANCE")

class BaseControl : public BaseWindow
{

protected:
	virtual LRESULT CALLBACK wndProc(UINT, WPARAM, LPARAM);
public:
	BaseControl(HINSTANCE hInst,int x,int y,int w,int h,HWND parent,LPCWSTR title, LPCWSTR className, UINT extraStyle);
};