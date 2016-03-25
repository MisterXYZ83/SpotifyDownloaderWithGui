

#include "SpotifyGuiController.h"
#include "spotify_downloader.h"
#include <tchar.h>

#define LOGBUTTON_ID	1
#define LOGREPORT_ID	2
#define PLAYLIST_ID		3
#define TRACK_ID		4


SpotifyGuiController::SpotifyGuiController(void *userdata)
{
	spotify_userdata = userdata;

	if (!spotify_userdata) return;
	
	SpotifyUserData *data = (SpotifyUserData *)spotify_userdata;
	data->guiController = this;

	//creo la gui
	instance = GetModuleHandle(0);

	isPlaylistCallbackSetted = 0;
	
	win = new MyWindow(instance);
	
	selected_playlist = 0;
	selected_track = 0;

	playlistcontainer_cb = (sp_playlistcontainer_callbacks *)malloc(sizeof(sp_playlistcontainer_callbacks));
	memset(playlistcontainer_cb, 0, sizeof(sp_playlistcontainer_callbacks));

	playlistcontainer_cb->container_loaded = SpotifyGuiController::container_loaded;
	playlistcontainer_cb->playlist_added = SpotifyGuiController::playlist_added;
	playlistcontainer_cb->playlist_moved = SpotifyGuiController::playlist_moved;
	playlistcontainer_cb->playlist_removed = SpotifyGuiController::playlist_removed;

	playlist_cb = (sp_playlist_callbacks *)malloc(sizeof(sp_playlist_callbacks));
	memset(playlist_cb, 0, sizeof(sp_playlist_callbacks));

	playlist_cb->tracks_added = SpotifyGuiController::tracks_added;
	playlist_cb->tracks_removed = SpotifyGuiController::tracks_removed;
	playlist_cb->tracks_moved = SpotifyGuiController::tracks_moved;
	playlist_cb->playlist_state_changed = SpotifyGuiController::playlist_state_changed;
	playlist_cb->playlist_renamed = SpotifyGuiController::playlist_renamed;
	playlist_cb->playlist_state_changed = SpotifyGuiController::playlist_state_changed;
	playlist_cb->playlist_update_in_progress = SpotifyGuiController::playlist_update_in_progress;
	playlist_cb->playlist_metadata_updated = SpotifyGuiController::playlist_metadata_updated;
	playlist_cb->track_created_changed = SpotifyGuiController::track_created_changed;
	playlist_cb->track_seen_changed = SpotifyGuiController::track_seen_changed;
	playlist_cb->description_changed = SpotifyGuiController::description_changed;
	playlist_cb->image_changed = SpotifyGuiController::image_changed;
	playlist_cb->track_message_changed = SpotifyGuiController::track_message_changed;
	playlist_cb->subscribers_changed = SpotifyGuiController::subscribers_changed;


	if ( win )
	{

		win->setBaseListener(this);
		win->setAsMainWindow(TRUE);
		win->setCaption(TEXT("Spotify Downloader"));
		
		win->show(1);

		//playlist
		playlistTree = new MyTreeView(instance, win->getHWND(), 10, 10, 330, 400);

		if ( playlistTree )
		{
			playlistTree->setID(PLAYLIST_ID);
			playlistTree->setEnableState(TRUE);
			playlistTree->setBaseListener(this);
		}

		trackTree = new MyTreeView(instance, win->getHWND(), 350, 10, 330, 400);

		if ( trackTree )
		{
			trackTree->setID(TRACK_ID);
			trackTree->setEnableState(TRUE);
			trackTree->setBaseListener(this);
		}


		logWin = new MyReport(instance, win->getHWND(), 10, 420, 780, 160);

		if ( logWin )
		{

			logWin->setID(LOGREPORT_ID);
			logWin->addRow(TEXT("Download delle canzoni Spotify!"));
			logWin->setBaseListener(this);
		}

		logButton = new MyButton(instance, win->getHWND(), 690, 10, 100, 40);

		if ( logButton )
		{
			logButton->setCaption(TEXT("Login"));
			logButton->setEnableState(TRUE);
			logButton->setID(LOGBUTTON_ID);
			logButton->setBaseListener(this);
		}
	}
}

LRESULT SpotifyGuiController::executeMessage(UINT idcontrol, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SpotifyUserData *data = (SpotifyUserData *)spotify_userdata;

	switch ( idcontrol )
	{
		case LOGBUTTON_ID:
		{
			if ( data && data->spotify_logged_in )
			{
				//logout
				int ret = SpotifyLogOut(data);

				if ( ret )
				{
					//comando inviato, attendo
					logButton->setEnableState(FALSE);
				}
			}
			else
			{
				int ret = SpotifyLogIn((SpotifyUserData *)data, "", "");

				if ( ret )
				{
					//comando inviato attendo
					logButton->setEnableState(FALSE);
				}
			}
		
		}
		break;
		
		case PLAYLIST_ID:
		{
			if ( WM_NOTIFY == msg && TVN_SELCHANGED == ((LPNMHDR)lParam)->code )
			{
				if ( playlistTree )
				{
					HTREEITEM item = playlistTree->getSelected();

					if ( item )
					{
						sp_playlist *plist = (sp_playlist *)playlistTree->getExtraData(item);

						if ( plist )
						{
							RefreshPlaylistTracks(plist);

							selected_playlist = plist;
						}
					}

				}
			}
		}
		break;

		case TRACK_ID:
		{
			if ( WM_NOTIFY == msg && TVN_SELCHANGED == ((LPNMHDR)lParam)->code )
			{
				if ( trackTree )
				{
					HTREEITEM item = trackTree->getSelected();

					if ( item )
					{
						sp_track *track = (sp_track *)playlistTree->getExtraData(item);

						

						selected_track = track;
					}

				}
			}
		}
		break;

	}

	return 0;
}

sp_track *SpotifyGuiController::GetCurrentTrack()
{
	sp_track *s_track = 0;

	if ( selected_playlist && selected_track )
	{
		s_track = selected_track;
	}
	else if ( selected_playlist )
	{
		//ritorno la prima traccia della playlist corrente

		if ( trackTree ) 
		{
			HTREEITEM root = trackTree->getRootItem();

			if ( root )
			{
				s_track = (sp_track *)trackTree->getExtraData(root);
				
				trackTree->selectItem(root);
			}
		}
	}

	if ( s_track )
	{
		if ( logWin )
		{
			logWin->addRow(TEXT("Playing track: %hs"), sp_track_name(s_track));
		}
	}
	
	return s_track;
}

void SpotifyGuiController::RefreshPlaylistTracks ( sp_playlist *plist )
{
	if ( plist && plist != selected_playlist )
	{

		if ( trackTree ) trackTree->removeAllItem();

		//ho la playlist, carico le tracce
		sp_playlist_add_callbacks(plist, playlist_cb, this);

		if ( sp_playlist_is_loaded(plist) )
		{
			//aggiungo tracce
			int n_tracks = sp_playlist_num_tracks(plist);

			for ( int nt = 0 ; nt < n_tracks ; nt++ )
			{
				sp_track *track = sp_playlist_track(plist, nt);

				if ( track && sp_track_is_loaded(track) )
				{
					if ( trackTree )
					{
						TCHAR trackname[1001] = {0};
						memset(trackname, 0, sizeof(TCHAR)*1001);

						_sntprintf_s(trackname, 1000, TEXT("%hs"), sp_track_name(track));
						trackTree->addItemAsLast(NULL, trackname, track, 0);
					}
				}
			}
		}

	}

}

sp_track *SpotifyGuiController::GetNextTrack()
{

	return 0;
}

void SpotifyGuiController::UpdateMetadata ( )
{
	//aggiorno le playlist e le tracce
	if ( logWin ) logWin->addRow(TEXT("Metadata Updated..."));

	//RefreshPlaylistContainer();
}

void SpotifyGuiController::LoggedIn ()
{
	if ( logButton )
	{
		logButton->setEnableState(TRUE);
		logButton->setCaption(TEXT("Logout"));
	}

	if ( logWin )
	{
		logWin->addRow(TEXT("Spotify Successful Logged IN"));
	}

	//avvio caricamento metadati

	SpotifyUserData *data = (SpotifyUserData *)spotify_userdata;

	if ( data->spotify )
	{
		data->container = sp_session_playlistcontainer(data->spotify);
		
		if ( logWin ) logWin->addRow(TEXT("Ricezione informazioni...."));
		
		if (data->container && !isPlaylistCallbackSetted )
		{
			sp_playlistcontainer_add_callbacks(data->container, playlistcontainer_cb, this);

			isPlaylistCallbackSetted = 1;

			//RefreshPlaylistContainer();
		}
		//else: attendo ancora

	}
	
}

void SpotifyGuiController::AddPlaylist( sp_playlist *plist)
{
	if (plist )
	{
		//aggiungo
		if (sp_playlist_is_loaded(plist) && playlistTree)
		{
			TCHAR plist_name[1001] = { 0 };
			_sntprintf_s(plist_name, 1000, TEXT("%hs"), sp_playlist_name(plist));

			playlistTree->addItemAsLast(NULL, plist_name, plist, 0);

			//sp_playlist_add_ref(plist);
		}
	}
}

void SpotifyGuiController::RefreshPlaylistContainer()
{
	SpotifyUserData *data = (SpotifyUserData *)spotify_userdata;

	if ( !data->container) return ;

	if ( sp_playlistcontainer_is_loaded(data->container) )
	{
		int n_plist = sp_playlistcontainer_num_playlists(data->container);

		for ( int np = 0 ; np < n_plist ; np++ )
		{
			sp_playlist *plist = sp_playlistcontainer_playlist(data->container, np);

			AddPlaylist(plist);
		}
	}
}

void SpotifyGuiController::LoggedOut ()
{
if ( logButton )
	{
		logButton->setEnableState(TRUE);
		logButton->setCaption(TEXT("Login"));
	}

	if ( logWin )
	{
		logWin->addRow(TEXT("Spotify Successful Logged OUT"));
	}

	if ( playlistTree )
	{
		playlistTree->removeAllItem();
	}

	if ( trackTree )
	{
		trackTree->removeAllItem();
	}
}

void SpotifyGuiController::StartPlaying()
{
	if  ( logButton ) logButton->setEnableState(FALSE);
	if ( playlistTree ) playlistTree->setEnableState(FALSE);
	if ( trackTree ) trackTree->setEnableState(FALSE);

	if ( logWin )
	{
		if ( selected_track ) logWin->addRow(TEXT("Playing %hs..."), sp_track_name(selected_track));
		else logWin->addRow(TEXT("Playing, no track selected..."));
	}
}

void SpotifyGuiController::StopPlaying()
{
	if  ( logButton ) logButton->setEnableState(TRUE);
	if ( playlistTree ) playlistTree->setEnableState(TRUE);
	if ( trackTree ) trackTree->setEnableState(TRUE);

	if ( logWin ) logWin->addRow(TEXT("Stopped.."));
}

void SpotifyGuiController::LogMessage ( char *message )
{
	if ( message )
	{
		if ( logWin )
		{
			logWin->addRow(TEXT("%hs"), message);
		}

		free(message);
		message = 0;
	}
}

HWND SpotifyGuiController::GetMainWindow()
{
	HWND ret = 0;

	if ( win ) ret = win->getHWND();

	return ret;
}

/////playlist callbacks

void __stdcall SpotifyGuiController::tracks_added(sp_playlist *pl, sp_track *const *tracks, int num_tracks, int position, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::playlist_renamed(sp_playlist *pl, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::playlist_state_changed(sp_playlist *pl, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
	SpotifyUserData *sp_userdata = (SpotifyUserData *)instance->spotify_userdata;

	//playlist caricata effettivamente, posso aggiungere
	instance->AddPlaylist(pl);
}

void __stdcall SpotifyGuiController::playlist_update_in_progress(sp_playlist *pl, bool done, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::playlist_metadata_updated(sp_playlist *pl, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::track_created_changed(sp_playlist *pl, int position, sp_user *user, int when, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::track_seen_changed(sp_playlist *pl, int position, bool seen, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::description_changed(sp_playlist *pl, const char *desc, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::image_changed(sp_playlist *pl, const byte *image, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::track_message_changed(sp_playlist *pl, int position, const char *message, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}

void __stdcall SpotifyGuiController::subscribers_changed(sp_playlist *pl, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
}


///////////////////// playlist container callbacks
void __stdcall SpotifyGuiController::playlist_added(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;

	if (instance->logWin) instance->logWin->addRow(TEXT("Playlist Received...."));

	//aggiungo la callback per la playlist
	sp_playlist_add_callbacks(playlist, instance->playlist_cb, instance);
}

void __stdcall SpotifyGuiController::playlist_removed(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;

	if (instance->logWin) instance->logWin->addRow(TEXT("Playlist Removed..."));

}

void __stdcall SpotifyGuiController::playlist_moved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, int new_position, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;

	if (instance->logWin) instance->logWin->addRow(TEXT("Playlist Moved..."));
}

void __stdcall SpotifyGuiController::container_loaded(sp_playlistcontainer *pc, void *userdata)
{
	SpotifyGuiController *instance = (SpotifyGuiController *)userdata;
	SpotifyUserData *sp_userdata = (SpotifyUserData *)instance->spotify_userdata;

	if (instance->logWin) instance->logWin->addRow(TEXT("Playlist Container Loaded..."));


	//il playlist container e' caricato, posso assegnare alle playlist le callbacks
	if (pc != sp_userdata->container) return; ///error...
	if (!sp_playlistcontainer_is_loaded(sp_userdata->container)) return; //aspettare....

	instance->RefreshPlaylistContainer();
}
