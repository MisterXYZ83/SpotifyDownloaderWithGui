#pragma once

#include <Windows.h>
#include <libspotify\api.h>
#include "DynamicBuffer.h"
#include "SpotifyGuiController.h"

//session callbacks
void __stdcall logged_in(sp_session *session, sp_error error);
void __stdcall logged_out(sp_session *session);
void __stdcall metadata_updated(sp_session *session);
void __stdcall connection_error(sp_session *session, sp_error error);
void __stdcall message_to_user(sp_session *session, const char *message);
void __stdcall notify_main_thread(sp_session *session);
int __stdcall  music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames);
void __stdcall play_token_lost(sp_session *session);
void __stdcall log_message(sp_session *session, const char *data);
void __stdcall end_of_track(sp_session *session);
void __stdcall streaming_error(sp_session *session, sp_error error);
void __stdcall userinfo_updated(sp_session *session);
void __stdcall start_playback(sp_session *session);
void __stdcall stop_playback(sp_session *session);
void __stdcall get_audio_buffer_stats(sp_session *session, sp_audio_buffer_stats *stats);
void __stdcall offline_status_updated(sp_session *session);
void __stdcall offline_error(sp_session *session, sp_error error);
void __stdcall credentials_blob_updated(sp_session *session, const char *blob);
void __stdcall connectionstate_updated(sp_session *session);
void __stdcall scrobble_error(sp_session *session, sp_error error);

static LRESULT CALLBACK SpotifyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static DWORD __stdcall SpotifyMainProc(LPVOID data);

#define MY_USERNAME  "empty"
#define MY_PASSWORD  "empty"

//#define ENABLE_TERMINAL 

#define MESSAGE_WINDOW_CLASS_NAME_SPOTIFY   TEXT("SPOTIFY_MSG_ONLY_CLASS")
#define MAX_USERNAME_LEN    100
#define MAX_PASSWORD_LEN    100

#define IDVAR_USERNAME  1
#define IDVAR_PASSWORD  2
#define IDVAR_LOGGED    3
#define IDVAR_LOGBUTTON 4
#define IDVAR_RANDOM_TRACK 5
#define IDVAR_RANDOM_TRACKNAME 6

#define LOGIN_STRING    "Logged In"
#define LOGOUT_STRING   "Logged Out"

#define METADATA_STRING_OK  "Ready"
#define METADATA_STRING_NO  "Off"

#define BUTTON_LOGIN_STRING "Login/Logout"
//messaggi

#define SPOTIFY_LOGGED_IN               WM_APP + 1
#define SPOTIFY_LOGGED_OUT              WM_APP + 2
#define SPOTIFY_METADATA_UPDATED        WM_APP + 3
#define SPOTIFY_CONNECTION_ERROR        WM_APP + 4
#define SPOTIFY_MESSAGE_USER            WM_APP + 5
#define SPOTIFY_MUSIC_DELIVERY          WM_APP + 6
#define SPOTIFY_PLAYTOKEN_LOST          WM_APP + 7
#define SPOTIFY_END_OF_TRACK            WM_APP + 8
#define SPOTIFY_LOG_MESSAGE             WM_APP + 9
#define SPOTIFY_STREAMING_ERROR         WM_APP + 10
#define SPOTIFY_USERINFO_UPDATED        WM_APP + 11
#define SPOTIFY_START_PLAYBACK          WM_APP + 12
#define SPOTIFY_STOP_PLAYBACK           WM_APP + 13
#define SPOTIFY_GET_AUDIOBUFFER_STATS   WM_APP + 14
#define SPOTIFY_OFFLINE_STATUS_UPDATED  WM_APP + 15
#define SPOTIFY_OFFLINE_ERROR           WM_APP + 16
#define SPOTIFY_CREDENTIALBLOB_UPDATED  WM_APP + 17
#define SPOTIFY_CONNECTIONSTATE_UPDATED WM_APP + 18
#define SPOTIFY_SCROBBLE_ERROR          WM_APP + 20
#define SPOTIFY_CLOSE_NUTS              WM_APP + 50

struct SpotifyUserData
{

	HANDLE		spotify_main_thread;
	DWORD		spotify_main_thread_id;

	CRITICAL_SECTION	spotify_lock;
	CONDITION_VARIABLE	spotify_cond;
	CONDITION_VARIABLE	spotify_close_cond;

	int					spotify_flag;
	int					spotify_close_flag;

	HWND				spotify_window;


	sp_playlistcontainer *container;
	sp_track *track;

	int metadata_loaded;

	CRITICAL_SECTION	spotify_buffer_lock;
	CONDITION_VARIABLE	spotify_buffer_cond;
	int spotify_logged_in;

	//DynamicBuffer *buffer;
	int last_bufferoverrun;

	int					spotify_timeout;

	WNDCLASSEX			message_class;

	sp_session	*spotify;
	sp_session_config spotify_config;
	sp_session_callbacks spotify_session_cb;

	DynamicBuffer			*buffer;

	int spotify_active;

	SpotifyGuiController *guiController;
};



bool CreateSpotifySession(SpotifyUserData **data);
bool OpenSpotifySession(SpotifyUserData *data);
bool CloseSpotifySession(SpotifyUserData *data);

int SpotifyLogIn(SpotifyUserData *data, char *username, char *password);
int SpotifyLogOut(SpotifyUserData *data);