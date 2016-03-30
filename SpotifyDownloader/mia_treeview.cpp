/*class MyTreeView {

	HWND hwnd;
	HWND parent;
	HINSTANCE inst;

public:
	MyTreeView ( HINSTANCE hinst, HWND parent );
	
	int addItemAsFirst( HTREEITEM parent, LPWSTR title, void *extradata);
	int addItemAsLast( HTREEITEM parent, LPWSTR title, void *extradata);

};*/

#include "mia_treeview.h"

MyTreeView::MyTreeView( HINSTANCE hinst, HWND parent, int x0, int y0, int w, int h )
:BaseControl(hinst,x0,y0,w,h,parent,TEXT("GenericList"),WC_TREEVIEW,TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_SINGLEEXPAND){
	//this->inst = hinst;
	//this->parent = parent;
	//this->x0 = x0;
	//this->y0 = y0;
	//this->w = w;
	//this->h = h;
	this->setBaseListener(0);
	this->imgList = NULL;
}

void MyTreeView::show(){
	/*this->hwnd = CreateWindowEx(0,WC_TREEVIEW,TEXT("TreeView"),
		WS_VISIBLE|WS_CHILD|WS_BORDER|TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_SINGLEEXPAND,
	   this->x0,this->y0,
	   this->w,this->h,this->parent,NULL,this->inst,this);*/
}

int MyTreeView::getItemCount(){
	return SendMessage(this->getHWND(),TVM_GETCOUNT,0,0);
}

BOOL MyTreeView::getNextItem( HTREEITEM elem, TVITEM *item ){
	if ( !elem || !item ) return FALSE;
	HTREEITEM nxt = (HTREEITEM)SendMessage(this->getHWND(),TVM_GETNEXTITEM,TVGN_NEXT,(LPARAM)elem);
	if ( nxt ){
		return this->getItem(nxt,item);
	}else return FALSE;
}

HTREEITEM MyTreeView::getNextHItem(HTREEITEM elem) {
	if (!elem) return FALSE;
	HTREEITEM nxt = (HTREEITEM)SendMessage(this->getHWND(), TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)elem);
	return nxt;
}

HIMAGELIST MyTreeView::setImageList( HBITMAP defIcon ){
	//if ( !defImage ) return 0;
	this->imgList = ImageList_Create(MYTREEVIEW_DEFAULT_ICONSIZE,MYTREEVIEW_DEFAULT_ICONSIZE,ILC_COLOR24,0,0);

	if ( defIcon && imgList )
	{
		defIconIdx = ImageList_Add(this->imgList, defIcon, NULL);
	}

	SendMessage(this->getHWND(),TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)this->imgList);
	/*HBITMAP image = (HBITMAP)LoadImage(this->hInstance, defImage, IMAGE_BITMAP, MYTREEVIEW_DEFAULT_ICONSIZE, MYTREEVIEW_DEFAULT_ICONSIZE,
                LR_DEFAULTSIZE | LR_LOADFROMFILE );
	if ( image ){
		ImageList_Add(this->imgList,image,NULL);
	}*/
	return 0;
}

HTREEITEM MyTreeView::addItem( HTREEITEM parent, LPWSTR title, void *extradata, HTREEITEM pos, HBITMAP icon ){
	//if ( !extradata ) return NULL;
	TVITEM item;
	TVINSERTSTRUCT tvIns;
	int idImg = 0;
	BOOL defImg = TRUE;
	if ( icon ) {
		//inserisco nella lista l'icona
		idImg = ImageList_Add(this->imgList,icon,NULL);
		if ( idImg != -1 ){
			item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			item.iImage = idImg;
			item.iSelectedImage = idImg;
			defImg = FALSE;
		}
	}

	if ( defImg ){
		/*HBITMAP image = (HBITMAP)LoadImage(this->hInstance, TEXT("question.bmp"), IMAGE_BITMAP, MYTREEVIEW_DEFAULT_ICONSIZE, MYTREEVIEW_DEFAULT_ICONSIZE,
                LR_DEFAULTSIZE | LR_LOADFROMFILE );*/
		//if ( image ){
			//idImg = ImageList_Add(this->imgList,image,NULL);
			item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			item.iImage = defIconIdx;
			item.iSelectedImage = defIconIdx;
		//}
	}
	item.lParam = (LPARAM)extradata;
	item.pszText = title;
	item.cchTextMax = sizeof(title)/sizeof(title[0]);
	
	tvIns.item = item;
	tvIns.hParent = parent;
	tvIns.hInsertAfter = pos;

	return (HTREEITEM)SendMessage(this->hWnd,TVM_INSERTITEM,0,(LPARAM)&tvIns);
}

HTREEITEM MyTreeView::addItemAsLast( HTREEITEM parent, LPWSTR title, void *extradata, HBITMAP item ){
	return this->addItem(parent,title,extradata,TVI_LAST, item);
}

HTREEITEM MyTreeView::addItemAsFirst( HTREEITEM parent, LPWSTR title, void *extradata, HBITMAP item ){
	return this->addItem(parent,title,extradata,TVI_FIRST, item);
}

void MyTreeView::expandNode(HTREEITEM elem, BOOL child){
	if ( elem ){
		SendMessage(this->hWnd,TVM_EXPAND,TVE_EXPAND,(LPARAM)elem);
		if ( child ){
			//espando anche i figli
			HTREEITEM firstChild = (HTREEITEM)SendMessage(this->hWnd,TVM_GETNEXTITEM,TVGN_CHILD,(LPARAM)elem);
			if ( firstChild ){
				//ok ha figli, richiamo ricorsivamente il figlio
				this->expandNode(firstChild,TRUE);
			}else{
				//ricavo il successivo nodo e richiamo
				HTREEITEM nextNode = (HTREEITEM)SendMessage(this->hWnd,TVM_GETNEXTITEM,TVGN_NEXT,(LPARAM)elem);
				this->expandNode(nextNode,TRUE);
			}
		}
	}
}

void MyTreeView::expandList(){
	HTREEITEM rootItem = (HTREEITEM)SendMessage(this->hWnd,TVM_GETNEXTITEM,TVGN_ROOT,NULL);
	this->expandNode(rootItem,TRUE);
}

BOOL MyTreeView::getItem( HTREEITEM elem, TVITEM *item ){
	if (elem && item){
		item->hItem = elem;
		item->mask = TVIF_PARAM | TVIF_IMAGE;
		return SendMessage(this->hWnd,TVM_GETITEM,0,(LPARAM)item);
	}
	return FALSE;
}

HTREEITEM MyTreeView::getItemWithRoutine(HTREEITEM parent, IsEqualRoutine routine, void *obj ){
	if ( parent && routine && obj ){
		//ottengo il valore di LPARAM dell'oggetto corrente
		TVITEM item;
		item.mask = TVIF_PARAM;
		item.hItem = parent;
		if ( !SendMessage(this->hWnd,TVM_GETITEM,0,(LPARAM)&item) ) return NULL;
		//oggetto trovato, abbiamo l'param
		if ( routine((void*)item.lParam,obj) ) {
			return item.hItem;
		}
		//non sono uguali, procedo coi figli
		HTREEITEM firstChild = (HTREEITEM)SendMessage(this->hWnd,TVM_GETNEXTITEM,TVGN_CHILD,(LPARAM)parent);
		if ( firstChild ){
			//ok ha figli, richiamo ricorsivamente il figlio
			HTREEITEM tmp = this->getItemWithRoutine(firstChild,routine,obj);
			if ( tmp ) return tmp;
		}
		//ricavo il successivo nodo e richiamo
		HTREEITEM nextNode = (HTREEITEM)SendMessage(this->hWnd,TVM_GETNEXTITEM,TVGN_NEXT,(LPARAM)parent);
		if ( nextNode ) {
			HTREEITEM tmp = this->getItemWithRoutine(nextNode,routine,obj);
			if ( tmp ) return tmp;
		}
	}
	return NULL;
}

HTREEITEM MyTreeView::getRootItem(){
	HTREEITEM rootItem = (HTREEITEM)SendMessage(this->hWnd,TVM_GETNEXTITEM,TVGN_ROOT,NULL);
	return rootItem;
}



BOOL MyTreeView::setItemIcon(HTREEITEM elem, HBITMAP icon)
{
	if ( !icon ) return FALSE;

	TVITEM item;
	memset(&item, 0, sizeof(TVITEM));
	BOOL ret = FALSE;

	if ( getItem(elem, &item) )
	{
		//cambio immagine
		int idimg = ImageList_Add(imgList, icon, NULL);	
		if ( idimg != -1 )
		{
			//elimino la vecchia immagine
			//La ImageList_Remove rimuove un'immagine dalla lista ma riordina gli indici delle immagini rimaste
			//in modo da essere sempre un intervallo senza buchi
			/*if ( item.iImage )
			{
				//l'immagine di default ha indice 0, le altre possono essere rimosse
				ImageList_Remove(imgList, item.iImage);
			}
			*/

			item.mask = TVIF_SELECTEDIMAGE | TVIF_IMAGE;
			item.iImage = idimg;
			item.iSelectedImage = idimg;

			ret = SendMessage(hWnd, TVM_SETITEM, 0, (LPARAM)&item);
		}
	}
	return ret;
}

BOOL MyTreeView::setItemDefaultIcon(HTREEITEM elem)
{
	TVITEM item;
	memset(&item, 0, sizeof(TVITEM));
	BOOL ret = FALSE;

	if ( getItem(elem, &item) )
	{
		//cambio immagine

		item.mask = TVIF_SELECTEDIMAGE | TVIF_IMAGE;
		item.iImage = 0;
		item.iSelectedImage = 0;

		ret = SendMessage(hWnd, TVM_SETITEM, 0, (LPARAM)&item);
	}
	return ret;
}

/*
HWND MyTreeView::getHWND(){
	return this->hWnd;
}
*/

BOOL MyTreeView::removeItem(HTREEITEM elem){
	if ( elem ){
		//rimuovo l'icona
		TVITEM item;
		item.hItem = elem;
		item.mask = TVIF_IMAGE;
		if ( SendMessage(this->hWnd,TVM_GETITEM,0,(LPARAM)&item) ){
			//rimuovo l'immagine se c'è
			ImageList_Replace(this->imgList,item.iImage,NULL,NULL);
			return SendMessage(this->hWnd,TVM_DELETEITEM,0,(LPARAM)elem);
		}
	}
	return FALSE;
}

LRESULT MyTreeView::executeMessage(UINT idcontrol, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	return CallWindowProc(this->getOldProc(),this->getHWND(),msg,wParam,lParam);
}

HTREEITEM MyTreeView::getSelected(){
	TVITEM item;
	return (HTREEITEM)SendMessage(this->getHWND(),TVM_GETNEXTITEM,TVGN_CARET,(LPARAM)&item);
}

void *MyTreeView::getExtraData ( HTREEITEM item ){
	if ( !item ) return NULL;
	TVITEM obj;
	obj.hItem = item;
	obj.mask = TVIF_PARAM;
	SendMessage(this->getHWND(),TVM_GETITEM,0,(LPARAM)&obj);
	return (void*)obj.lParam;
}

void MyTreeView::selectItem( HTREEITEM item ){
	SendMessage(this->getHWND(),TVM_SELECTITEM,TVGN_CARET,(LPARAM)item);
}

BOOL MyTreeView::removeAllItem(){
	return SendMessage(this->getHWND(),TVM_DELETEITEM,0,(LPARAM)TVI_ROOT);
}