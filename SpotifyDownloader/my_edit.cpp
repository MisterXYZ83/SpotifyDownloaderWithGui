
#include "my_edit.h"

MyEdit::MyEdit( HINSTANCE hinst, HWND parent, int x0, int y0, int w, int h )
:BaseControl(hinst,x0,y0,w,h,parent,TEXT("Inserire qui il testo..."),TEXT("EDIT"), 0 )
{
	//this->inst = hinst;
	//this->parent = parent;
	//this->x0 = x0;
	//this->y0 = y0;
	//this->w = w;
	//this->h = h;
	HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(this->getHWND(),WM_SETFONT,(WPARAM)font,MAKELPARAM(1,0));
}
