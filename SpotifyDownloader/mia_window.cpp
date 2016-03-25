
#include "mia_window.h"
#include <stdio.h>

#define MY_WINDOW_CLASSNAME "MyWindow"

#define NOTIFY_CHILD_CREATION		WM_USER + 1

const TCHAR MyWindow::WindowClass[] = TEXT(MY_WINDOW_CLASSNAME);


MyWindow::MyWindow(HINSTANCE h, int x0, int y0,
						  int lx, int ly, 
						  UINT wStyle,
						  UINT style,
						  HBRUSH back,
						  HWND parent,
						  BOOL isMain)
{
  
   WNDCLASS tmp;
   this->idWin = 0;
   this->isMainFlag = isMain;
   this->graphRoutine = NULL;
   this->parentHwnd = parent;
   this->closeList = 0;
   this->listener = NULL;

   if ( !GetClassInfo(h,MyWindow::WindowClass,&tmp) )
   {

		wndClass.style          = style;
		wndClass.lpfnWndProc    = BaseWindow::baseWndProc;
		wndClass.cbClsExtra     = 0;
		wndClass.cbWndExtra     = 0;
		wndClass.hInstance      = h;
		wndClass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground  = back;
		wndClass.lpszMenuName   = NULL;
		wndClass.lpszClassName  = MyWindow::WindowClass;
	
		RegisterClass(&wndClass);
   }

   RECT tmpRect;
   tmpRect.top = y0;
   tmpRect.left = x0;
   tmpRect.right = x0 + lx;
   tmpRect.bottom = y0 + ly;

   AdjustWindowRect(&tmpRect,wStyle,FALSE);

   //salvo l'handle della finestra
   this->hWnd = CreateWindow( 
      MyWindow::WindowClass,		// window class name
      TEXT(DEFAULT_CAPTION),		// window caption
      wStyle,						// window style
	  tmpRect.left,							// initial x position
	  tmpRect.top,							// initial y position
	  tmpRect.right - tmpRect.left,							// initial x size
	  tmpRect.bottom - tmpRect.top,							// initial y size
      parent,						// parent window handle
      NULL,							// window menu handle
      h,							// program instance handle
      this);						// creation parameters
}

BaseWindow::~BaseWindow()
{
	//deregistro la classe
	UnregisterClass(MyWindow::WindowClass, hInstance);
}

int BaseWindow::getID()
{
	return this->idWin;
}

BOOL BaseWindow::setID( UINT id )
{
	if ( this->idWin != 0 ) return FALSE;
	else
	{
		this->idWin = id;
		return TRUE;
	}
}

HWND BaseWindow::getParentHWND()
{
	return this->parentHwnd;
}

BOOL BaseWindow::setOnCloseListener( CloseListener *l )
{
	if ( l ) this->closeList = l;
	return TRUE;
}

LRESULT CALLBACK BaseWindow::baseWndProc(HWND hwnd, UINT mess, WPARAM wParam, LPARAM lParam)
{
	BaseWindow *instance = NULL;
	BaseWindow *childInstance = NULL;
	if ( mess == WM_CREATE )
	{
		//ottengo il puntatore this
		CREATESTRUCT *cStruct = (CREATESTRUCT*)lParam;
		instance = (BaseWindow*)cStruct->lpCreateParams;
		if ( instance )
			//salvo il puntatore nello spazio utente della window
			SetWindowLongPtr(hwnd,GWLP_USERDATA, (LONG_PTR)instance);
		return TRUE;
	}
	else
	{
		//verifico la notifica di un child
		if ( mess == NOTIFY_CHILD_CREATION )
		{
			//un child notifica la propria creazione
			HWND tmpHwnd = (HWND)wParam;
			childInstance = (BaseWindow*)lParam;
			if ( childInstance && tmpHwnd )
			{
				//salvo il this sullo spazio utente della hwnd
				SetWindowLongPtr(tmpHwnd, GWLP_USERDATA, (LONG_PTR)childInstance);
			}
		}
		else if ( mess == WM_COMMAND || mess == WM_HSCROLL || mess == WM_VSCROLL )
		{
			instance = BaseWindow::getInstance((HWND)lParam);
		}
		else if ( mess == WM_NOTIFY )
		{
			LPNMHDR data = (LPNMHDR)lParam;
			instance = BaseWindow::getInstance(data->hwndFrom);
		}
		else if ( mess == WM_MENUCOMMAND )
		{
			//messaggio da un menu con MF_BYPOSITION
			HMENU menu = (HMENU)lParam;
			MENUINFO info;
			memset(&info,0,sizeof(MENUINFO));
			info.fMask = MIM_MENUDATA;
			info.cbSize = sizeof(MENUINFO);
			GetMenuInfo(menu,&info);
			instance = (BaseWindow*)info.dwMenuData;
		} 
		/*else if ( mess == WM_CLOSE )
		{
			instance = BaseWindow::getInstance(hwnd);
			if ( instance && instance->closeList ) return instance->closeList->onClose(instance->idWin, instance->getHWND());
			
		} */
		else instance = BaseWindow::getInstance(hwnd); 
		
		if ( instance ) return instance->wndProc(mess, wParam, lParam);
		else return DefWindowProc(hwnd, mess, wParam, lParam);
	
	}
}

void BaseWindow::setBaseListener( BaseListener *listener )
{
	this->listener = listener;
}

BaseWindow *BaseWindow::getInstance(HWND hwnd)
{
	return (BaseWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

void MyWindow::show(int mode)
{
	ShowWindow(this->hWnd,mode);
}

HWND BaseWindow::getHWND()
{
	return this->hWnd;
}


void MyWindow::onDestroy()
{

}

LRESULT CALLBACK MyWindow::wndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
   HDC          hdc;
   PAINTSTRUCT  ps;
   BOOL ret;

   switch(message)
   {
   case WM_PAINT:
	   hdc = BeginPaint(this->hWnd, &ps);
		//disegno qualcosa...
	   if ( this->graphRoutine ) this->graphRoutine->onPaint(hdc, ps, NULL);
	   EndPaint(this->hWnd, &ps);
	   return 0;
   case WM_DESTROY:
	   this->onDestroy();
	   if ( this->isMainFlag ) PostQuitMessage(0);
	   return 0;
   default:
	   ret = DefWindowProc(this->hWnd, message, wParam, lParam);
	   if ( this->listener )
	   {

		   this->listener->executeMessage(getID(), getHWND(), message, wParam, lParam);
	   }
   }

   return ret;
}

void MyWindow::setAsMainWindow(BOOL flag)
{
	this->isMainFlag = flag;
}

BOOL MyWindow::isMain()
{
	return this->isMainFlag;
}

BOOL BaseWindow::setCaption(TCHAR *cap)
{
	if ( cap ) return SendMessage(this->hWnd, WM_SETTEXT, NULL, (LPARAM)cap);
	else return FALSE;
}

int BaseWindow::getCaption(TCHAR *string, int maxLen)
{
	if ( string ) return SendMessage(this->hWnd, WM_GETTEXT, maxLen, (LPARAM)string);
	else return -1;
}

void MyWindow::setPaintCallback( PaintRoutine *r )
{
	if ( r ) this->graphRoutine = r;
	else this->graphRoutine = NULL;
}

WNDPROC BaseWindow::getOldProc()
{
	return this->oldProc;
}

///////////////////////////////////////////////////

LRESULT CALLBACK BaseControl::wndProc(UINT mess, WPARAM wParam, LPARAM lParam)
{
	if ( this->listener ) return this->listener->executeMessage(this->idWin, this->hWnd, mess, wParam, lParam);
	else return CallWindowProc(this->oldProc, this->hWnd, mess, wParam, lParam);
	//return DefWindowProc(this->hWnd,mess,wParam,lParam);
}

BaseControl::BaseControl(HINSTANCE hInst, int x, int y, int w, int h, HWND parent, LPCWSTR title, LPCWSTR className, UINT extraStyle)
{
	UINT style = WS_VISIBLE | WS_CHILD | WS_BORDER | extraStyle;
	this->idWin = 0;
	this->listener = 0;
	this->parentHwnd = parent;
	this->hWnd = CreateWindow(className, title, style, x, y, w, h, parent, NULL, hInst, this);
	//SetProp(this->hWnd,TEXT(INSTANCE_PROP),(HANDLE)this);
	//invio un messaggio di notifica
	this->oldProc = (WNDPROC)GetWindowLongPtr(this->hWnd, GWLP_WNDPROC);
	SendMessage(this->getParentHWND(), NOTIFY_CHILD_CREATION, (WPARAM)this->hWnd, (LPARAM)this);
}

BOOL BaseWindow::setEnableState(BOOL val)
{
	return EnableWindow(this->getHWND(), val);
}