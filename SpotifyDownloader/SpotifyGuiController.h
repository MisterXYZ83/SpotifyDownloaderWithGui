#pragma once

#include "mia_window.h";
#include "mia_treeview.h";
#include "my_button.h";
#include "my_report.h";
#include "my_edit.h"
#include <libspotify\api.h>

class PlugIn;

class SpotifyGuiController : public BaseListener
{
	void *spotify_userdata;

	MyWindow	*win;
	MyTreeView	*playlistTree;
	MyTreeView	*trackTree;
	MyReport	*logWin;
	MyButton	*logButton;
	MyEdit		*usernameEdit;
	MyEdit		*passwordEdit;
	MyButton	*downloadButton;
	MyTreeView	*downloadListTree;

	HINSTANCE	instance;

	BOOL isPlaylistCallbackSetted;

	//sp_playlistcontainer_callbacks *playlistcontainer_cb;
	//sp_playlist_callbacks *playlist_cb;

	sp_playlist *selected_playlist;
	sp_track *selected_track;

	static void __stdcall playlist_added(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata);
	static void __stdcall playlist_removed (sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata);
	static void __stdcall playlist_moved (sp_playlistcontainer *pc, sp_playlist *playlist, int position, int new_position, void *userdata);
	static void __stdcall container_loaded (sp_playlistcontainer *pc, void *userdata);

	static void __stdcall tracks_added(sp_playlist *pl, sp_track *const *tracks, int num_tracks, int position, void *userdata);
	static void __stdcall tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata);
	static void __stdcall tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata);
	static void __stdcall playlist_renamed(sp_playlist *pl, void *userdata);
	static void __stdcall playlist_state_changed(sp_playlist *pl, void *userdata);
	static void __stdcall playlist_update_in_progress(sp_playlist *pl, bool done, void *userdata);
	static void __stdcall playlist_metadata_updated(sp_playlist *pl, void *userdata);
	static void __stdcall track_created_changed(sp_playlist *pl, int position, sp_user *user, int when, void *userdata);
	static void __stdcall track_seen_changed(sp_playlist *pl, int position, bool seen, void *userdata);
	static void __stdcall description_changed(sp_playlist *pl, const char *desc, void *userdata);
	static void __stdcall image_changed(sp_playlist *pl, const byte *image, void *userdata);
	static void __stdcall track_message_changed(sp_playlist *pl, int position, const char *message, void *userdata);
	static void __stdcall subscribers_changed(sp_playlist *pl, void *userdata);

	void RefreshPlaylistContainer();

	void RefreshPlaylistTracks( sp_playlist *plist );
	void AddPlaylist(sp_playlist *plist);

public:

	SpotifyGuiController (void *userdata);

	void UpdateMetadata ( );
	void LoggedIn (int err);
	void LoggedOut ();

	void StartPlaying();
	void StopPlaying();

	sp_track *GetCurrentTrack();
	sp_track *GetNextTrack();
	sp_track *GetFirstDownloadTrack();
	sp_track *GetNextDownloadTrack(sp_track *actual);

	void LogMessage ( char *message );


	virtual LRESULT executeMessage(UINT idcontrol, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND GetMainWindow();


};

