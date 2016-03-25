#include "windows.h"
#include "spotify_downloader.h"
#include "SpotifyGuiController.h"


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
{
	MSG msg;
	BOOL bRet;
	UNREFERENCED_PARAMETER(lpszCmdLine);


	//creo una sessione spotify
	SpotifyUserData *data = 0;
	if (!CreateSpotifySession(&data)) return -1;
	if (!OpenSpotifySession(data)) return -1;

	//creo un controller
	SpotifyGuiController *controller = new SpotifyGuiController(data);


	//loop messaggi
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	return msg.wParam;
}