#include "my_report.h"
#include <tchar.h>

MyReport::MyReport( HINSTANCE hinst, HWND parent, int x0, int y0, int w, int h )
:BaseControl(hinst,x0,y0,w,h,parent,TEXT("LogWindow"),WC_LISTVIEW,LVS_REPORT | LVS_SINGLESEL  )
{
	//this->inst = hinst;
	//this->parent = parent;
	//this->x0 = x0;
	//this->y0 = y0;
	//this->w = w;
	//this->h = h;
	HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(this->getHWND(),WM_SETFONT,(WPARAM)font,MAKELPARAM(1,0));
	this->setBaseListener(0);
	//Creo la colonna tempo
	LVCOLUMN column;
	column.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.cx = 100;
	column.pszText = TEXT("Time");
	column.cchTextMax = 100;
	SendMessage(this->getHWND(),LVM_INSERTCOLUMN,0,(LPARAM)&column);
	//creo la colonna messaggio
	column.cx = 400;
	column.pszText = TEXT("Messagges");
	SendMessage(this->getHWND(),LVM_INSERTCOLUMN,1,(LPARAM)&column);

	SendMessage(this->getHWND(),LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT);
	this->setBaseListener(0);
}

BOOL MyReport::addRow( TCHAR *format, ...)
{

#define MAX_MSG_LEN 15000


	if ( !format ) return FALSE;
	
	TCHAR *msg = (TCHAR *)malloc((MAX_MSG_LEN + 1) * sizeof(TCHAR));
	memset(msg, 0, (MAX_MSG_LEN + 1) * sizeof(TCHAR));

	//inserisco il submessage time
	LVITEM item;
	item.iItem = SendMessage(this->getHWND(),LVM_GETITEMCOUNT,0,0);

	item.iSubItem = 0;//time
	item.mask = LVIF_TEXT;
	item.state = 0;
	item.stateMask = 0;
	SYSTEMTIME time;
	GetLocalTime(&time);
	
	_stprintf_s(msg, (MAX_MSG_LEN), TEXT("%02d:%02d:%02d.%03d"), time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

	//item.pszText = msg->getBuffer();
	//item.cchTextMax = msg->getSize();
	item.pszText = _tcsdup(msg);
	item.cchTextMax = _tcslen(msg);

	memset(msg, 0, (MAX_MSG_LEN + 1) * sizeof(TCHAR));

	if ( SendMessage(this->getHWND(), LVM_INSERTITEM, 0, (LPARAM)&item) != -1 );
	else return FALSE;
	//inserisco messaggio testo
	item.iSubItem = 1;
	item.mask = LVIF_TEXT;
	item.state = 0;
	item.stateMask = 0;
	va_list args;
	va_start(args,format);
	
	//msg->appendDatavf(format,args);
	_vstprintf(msg, (MAX_MSG_LEN), format, args);
	
	//item.pszText = msg->getBuffer();
	//item.cchTextMax = msg->getSize();
	item.pszText = _tcsdup(msg);
	item.cchTextMax = _tcslen(msg);

	//delete msg;
	free(msg);
	msg = NULL;


	if ( SendMessage(this->getHWND(),LVM_SETITEM,0,(LPARAM)&item) )
	{
		int idx = SendMessage(this->getHWND(),LVM_GETITEMCOUNT,0,0);
		SendMessage(this->getHWND(),LVM_ENSUREVISIBLE,idx-1,TRUE);
	}
	else return FALSE;
}


BOOL MyReport::setCaption(TCHAR *caption)
{
	return TRUE;
}


int MyReport::getCaption(TCHAR *caption, int maxLen)
{
	return 0;
}



BOOL MyReport::removeAllRows ( )
{
	SendMessage(this->getHWND(),LVM_DELETEALLITEMS,0,0);
	return TRUE;
}