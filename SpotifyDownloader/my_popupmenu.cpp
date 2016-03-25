
#include "my_popupmenu.h"

BasePopupMenu::BasePopupMenu()
{
	this->menu = CreatePopupMenu();
	this->menuInfo.cbSize = sizeof(this->menuInfo);
	this->menuInfo.fMask = MIM_STYLE|MIM_MENUDATA;
	this->menuInfo.dwMenuData = (ULONG_PTR)this;//da vedere
	this->menuInfo.dwStyle = MNS_NOTIFYBYPOS;
	SetMenuInfo(menu,&this->menuInfo);
	this->lastItemPos = 0;
	this->listener = 0;
}

void BasePopupMenu::showMenu(int x, int y, HWND owner)
{
	TrackPopupMenuEx(this->menu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_LEFTBUTTON|TPM_HORPOSANIMATION,x,y,owner,NULL);
}

BOOL BasePopupMenu::insertTextItem(int idx, LPWSTR title, void *extraData)
{
	if ( idx <= 0 || !title ) return FALSE;
	//inserisco le informazioni per il singolo
	MENUITEMINFO info;
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_TYPE|MIIM_ID|MIIM_DATA;
	info.fType = MFT_STRING;
	info.dwTypeData = title;
	info.cch = sizeof(title);
	info.wID = idx;
	info.dwItemData = (ULONG_PTR)extraData; //da vedere cosa inserire
	return InsertMenuItem(this->menu,this->lastItemPos++,MF_BYPOSITION,&info);
}

BOOL BasePopupMenu::insertSeparator()
{
	MENUITEMINFO info;
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_FTYPE|MIIM_ID;
	info.fType = MFT_SEPARATOR;
	info.wID = 0;
	return InsertMenuItem(this->menu,this->lastItemPos++,MF_BYPOSITION,&info);//0 usato per i separatori
}

BOOL BasePopupMenu::insertBitmapItem(int idx, LPWSTR title, HBITMAP img, void *extraData)
{
	if ( idx <= 0 || !title || !img ) return FALSE;
	//inserisco le informazioni per il singolo
	MENUITEMINFO info;
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_ID|MIIM_DATA|MIIM_STRING|MIIM_BITMAP;
	//info.fType = MFT_BITMAP;
	info.dwTypeData = title;
	info.cch = sizeof(title);
	info.wID = idx;
	info.hbmpItem = img;
	//info.hbmpChecked = img;
	//info.hbmpUnchecked = img;
	info.dwItemData = (ULONG_PTR)extraData; //da vedere cosa inserire
	return InsertMenuItem(this->menu,this->lastItemPos++,MF_BYPOSITION,&info);
}

LRESULT CALLBACK BasePopupMenu::wndProc(UINT mess, WPARAM wParam, LPARAM lParam)
{
	//riceve il messaggio e lo manda al listener
	if ( this->listener ) return this->listener->dispatchCommand(this->menu, mess, wParam, lParam);
	return FALSE;
}

void BasePopupMenu::setBaseCommandListener( BaseMenuCommandListener *l )
{
	if ( l ) this->listener = l;
}

BasePopupMenu *BasePopupMenu::getMenu( HMENU h )
{
	if ( h ){
		MENUINFO info;
		memset(&info,0,sizeof(MENUINFO));
		info.cbSize = sizeof(MENUINFO);
		info.fMask = MIM_MENUDATA;
		GetMenuInfo(h,&info);
		return (BasePopupMenu*)info.dwMenuData;
	}
}