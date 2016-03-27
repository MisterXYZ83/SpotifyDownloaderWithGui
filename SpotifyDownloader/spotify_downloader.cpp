#include <Windows.h>
#include <stdio.h>

#include "spotify_downloader.h"

#include <time.h>
#include <math.h>


void CleanSpotifySession(SpotifyUserData *data);
LRESULT SpotifyHandleMessage(SpotifyUserData *user_data, UINT msg, WPARAM wparam, LPARAM lparam);
void SpotifyMainLoop(SpotifyUserData *user_data);


int PrepareEncoder(SpotifyUserData *context, const sp_audioformat *fmt);
int CloseEncoder(SpotifyUserData *context);
int FreeMP3Buffer(SpotifyUserData *context);
int AllocMp3Buffer(SpotifyUserData *context, int num_frames);
void track_ended(SpotifyUserData *context);


uint8_t *g_appkey;
size_t g_appkey_size;

extern "C"
{
	FILE __iob_func[3] = { *stdin,*stdout,*stderr };
}

bool CreateSpotifySession(SpotifyUserData **data)
{
	if (data == NULL) return NULL;

	//carico da file la key, nome utente e password
	char *key_file = "spotify_appkey.key";
	char cur_dir[50001];
	char full_path[100001];
	memset(full_path, 0, 100001);
	memset(cur_dir, 0, 50001);

	if (GetCurrentDirectoryA(50000, cur_dir))
	{
		size_t len_path = strnlen_s(cur_dir, 50000);
		
		strcpy_s(full_path, cur_dir);
		
		full_path[len_path] = '\\';
		
		strcpy_s(&full_path[len_path+1], 10000-len_path, key_file);
	}

	FILE *fp = NULL;
	fopen_s(&fp, full_path, "rb");

	if (!fp) return false;

	fseek(fp, 0, SEEK_END);
	g_appkey_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	g_appkey = (uint8_t*)malloc(g_appkey_size*sizeof(uint8_t));
	fread(g_appkey, 1, g_appkey_size, fp);

	fclose(fp);

	SpotifyUserData *user_data = NULL;
	//l'utente deve fornire un puntatore vuoto!

	if (*data != NULL) return false;
	
	//alloco la struttura
	*data = user_data = new SpotifyUserData;
	memset(user_data, 0, sizeof(SpotifyUserData));

	user_data->lame = lame_init();
	user_data->encoder_ready = 0;

	AllocMp3Buffer(user_data, 0);

	user_data->spotify_flag = 0;
	InitializeCriticalSection(&user_data->spotify_lock);
	InitializeConditionVariable(&user_data->spotify_cond);
	user_data->spotify_timeout = 0;

	InitializeCriticalSection(&user_data->spotify_buffer_lock);
	InitializeConditionVariable(&user_data->spotify_buffer_cond);
	InitializeConditionVariable(&user_data->spotify_close_cond);

	//creo una finestra soli messaggi per ricevere i messaggi da spotify
	//registro la classe

	memset(&user_data->message_class, 0, sizeof(WNDCLASSEX));
	user_data->message_class.cbSize = sizeof(WNDCLASSEX);
	user_data->message_class.lpfnWndProc = SpotifyWndProc;
	user_data->message_class.hInstance = GetModuleHandle(NULL);
	user_data->message_class.lpszClassName = MESSAGE_WINDOW_CLASS_NAME_SPOTIFY;

	if (RegisterClassEx(&user_data->message_class))
	{
		user_data->spotify_window = CreateWindowEx(0, MESSAGE_WINDOW_CLASS_NAME_SPOTIFY, MESSAGE_WINDOW_CLASS_NAME_SPOTIFY, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, user_data);
	}

	//creo la sessione spotify
	memset(&user_data->spotify_config, 0, sizeof(sp_session_config));
	memset(&user_data->spotify_session_cb, 0, sizeof(sp_session_callbacks));

	user_data->spotify_session_cb.connection_error = connection_error;
	user_data->spotify_session_cb.logged_in = logged_in;
	user_data->spotify_session_cb.logged_out = logged_out;
	user_data->spotify_session_cb.log_message = log_message;
	user_data->spotify_session_cb.music_delivery = music_delivery;
	user_data->spotify_session_cb.notify_main_thread = notify_main_thread;
	user_data->spotify_session_cb.start_playback = start_playback;
	user_data->spotify_session_cb.stop_playback = stop_playback;
	user_data->spotify_session_cb.end_of_track = end_of_track;

	user_data->spotify_config.api_version = SPOTIFY_API_VERSION;
	user_data->spotify_config.application_key = g_appkey;
	user_data->spotify_config.application_key_size = g_appkey_size;
	user_data->spotify_config.cache_location = "spotify_cache";
	user_data->spotify_config.settings_location = "spotify_cache";
	user_data->spotify_config.userdata = (void *)user_data;
	user_data->spotify_config.callbacks = &user_data->spotify_session_cb;
	user_data->spotify_config.user_agent = "SpotifyConsoleInterface";

}


bool OpenSpotifySession(SpotifyUserData *user_data)
{
	sp_error err = sp_session_create(&user_data->spotify_config, &user_data->spotify);

	if (SP_ERROR == err)
	{
		CleanSpotifySession(user_data);

		return false;
	}
	else
	{
		//creo il thread spotify
		user_data->spotify_main_thread = CreateThread(NULL, 0, SpotifyMainProc, user_data, 0, &user_data->spotify_main_thread_id);
		user_data->spotify_active = (user_data->spotify_main_thread != NULL);

		return user_data->spotify_active;
	}
}

int SpotifyLogIn(SpotifyUserData *data, char *username, char *password)
{
	int ret = 0;

	if (!data->spotify_logged_in)
	{
		//faccio il logout
		EnterCriticalSection(&data->spotify_lock);

		if (username && password)
		{
			sp_error err = sp_session_login(data->spotify, username, password, 0, 0);

			if (SP_ERROR_OK != err) ret = 0;
			else ret = 1;
		}

		//disattivo il bottone fino all'effettuato login

		LeaveCriticalSection(&data->spotify_lock);
	}

	return ret;
}

int SpotifyLogOut(SpotifyUserData *data)
{
	int ret = 0;

	if (data->spotify_logged_in)
	{
		//faccio il logout
		EnterCriticalSection(&data->spotify_lock);

		sp_error err = sp_session_logout(data->spotify);

		if (SP_ERROR_OK != err) ret = 0;
		else ret = 1;

		//disattivo il bottone fino all'effettuato login

		LeaveCriticalSection(&data->spotify_lock);
	}

	return ret;
}

bool CloseSpotifySession(SpotifyUserData *data)
{
	return false;
}


int SpotifyDownloadTrack(SpotifyUserData *instance, sp_track *track)
{

	if (instance && track)
	{
		//scarico la traccia

		char file_name[2048];
		memset(file_name, 0, 2047);
		
		instance->track = track;

		if (sp_track_is_loaded(instance->track))
		{
			//traccia caricata..posso preparare
			const char *track_name = sp_track_name(instance->track);

			_snprintf_s(file_name, 2047, "%s.mp3", track_name);

			instance->actual_download_track_is_single = 1;
			instance->actual_download_track_name = track_name;
			//instance->actual_file = new WaveFile(file_name);
			instance->actual_samples = 0;
			instance->track_total_samples = instance->track_total_ms = sp_track_duration(instance->track);
			instance->last_written_samples = 0;

			fopen_s(&instance->fp_mp3, file_name, "wb");

			if (!instance->fp_mp3)
			{
				//printf("Errore apertura file....\r\n");
				return -1;
			}

			//printf("Avvio download %s\r\n", track_name);

		}

		sp_error err = sp_session_player_load(instance->spotify, instance->track);

		if (err == SP_ERROR_OK)
		{
			instance->track_downloaded = 0;
			sp_session_player_play(instance->spotify, true);
		}
		else if (err == SP_ERROR_IS_LOADING)
		{
			return SP_ERROR_IS_LOADING;
		}
		else if (err == SP_ERROR_NO_STREAM_AVAILABLE)
		{
			return SP_ERROR_NO_STREAM_AVAILABLE;
		}

		return SP_ERROR_OK;
	}
	else return -1;
}


////////////////////////////////////////
//ROUTINE PRIVATE

void CleanSpotifySession(SpotifyUserData *data)
{


}




LRESULT CALLBACK SpotifyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//create
	LONG_PTR res = 0;

	if (WM_CREATE == uMsg)
	{
		CREATESTRUCT *params = (CREATESTRUCT *)lParam;
		SetWindowLong(hwnd, GWLP_USERDATA, (LONG)params->lpCreateParams);
	}
	else if (uMsg >= SPOTIFY_LOGGED_IN && uMsg <= SPOTIFY_DOWNLOAD_STATUS)
	{

		res = -1;

		SpotifyUserData *user_data  = (SpotifyUserData *)GetWindowLong(hwnd, GWLP_USERDATA);

		if (user_data)
		{
			//processing messaggi da spotify
			res = SpotifyHandleMessage(user_data, uMsg, wParam, lParam);
		}

		if (res >= 0)
		{
			return res;
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


LRESULT SpotifyHandleMessage(SpotifyUserData *user_data, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//processo i messaggi spotify
	switch (msg)
	{
	case SPOTIFY_LOGGED_IN:
	{
		user_data->spotify_logged_in = 1;
		
		//verifico i metadati

		EnterCriticalSection(&user_data->spotify_lock);

		if (user_data->guiController) user_data->guiController->LoggedIn(wparam);

		/*if ( user_data->container && !user_data->metadata_loaded )
		{
		sp_track *track = user_data->track;

		if ( !track )
		{
		track = PlugIn::GetRandomTrack(user_data->container);
		}

		if ( track )
		{
		//user_data->metadata_loaded = 1;
		user_data->track = track;

		if ( sp_track_is_loaded(track) )
		{
		user_data->metadata_loaded = 1;

		CBFunction(this, NUTS_UPDATERTWATCH, IDVAR_RANDOM_TRACK, 0);
		CBFunction(this, NUTS_UPDATERTWATCH, IDVAR_RANDOM_TRACKNAME, 0);
		}
		}
		}
		*/

		LeaveCriticalSection(&user_data->spotify_lock);
	}
	break;

	case SPOTIFY_METADATA_UPDATED:
	{

		EnterCriticalSection(&user_data->spotify_lock);

		if (user_data->guiController) user_data->guiController->UpdateMetadata();
		/*user_data->container = sp_session_playlistcontainer(spotify);

		if ( user_data->container && !user_data->metadata_loaded )
		{
		sp_track *track = user_data->track;

		if ( !track )
		{
		track = PlugIn::GetRandomTrack(user_data->container);
		}

		if ( track )
		{
		//user_data->metadata_loaded = 1;
		user_data->track = track;

		if ( sp_track_is_loaded(track) )
		{
		user_data->metadata_loaded = 1;

		CBFunction(this, NUTS_UPDATERTWATCH, IDVAR_RANDOM_TRACK, 0);
		CBFunction(this, NUTS_UPDATERTWATCH, IDVAR_RANDOM_TRACKNAME, 0);
		}
		}
		}*/

		LeaveCriticalSection(&user_data->spotify_lock);

	}
	break;

	case SPOTIFY_LOGGED_OUT:
	{
		user_data->spotify_logged_in = 0;
		
		EnterCriticalSection(&user_data->spotify_lock);

		sp_session_forget_me(user_data->spotify);

		user_data->track = 0;
		user_data->metadata_loaded = 0;
		user_data->container = 0;

		if (user_data->guiController) user_data->guiController->LoggedOut();

		LeaveCriticalSection(&user_data->spotify_lock);

		//CBFunction(this, NUTS_UPDATERTWATCH, IDVAR_LOGGED, 0);
	}
	break;

	case SPOTIFY_CLOSE_NUTS:
	{
		EnterCriticalSection(&user_data->spotify_lock);

		user_data->spotify_flag = 1;
		user_data->spotify_timeout = 0;
		user_data->spotify_active = 0;

		WakeAllConditionVariable(&user_data->spotify_cond);


		LeaveCriticalSection(&user_data->spotify_lock);
	}
	break;

	case SPOTIFY_LOG_MESSAGE:
	{
		if (user_data->guiController)
		{
			user_data->guiController->LogMessage((char *)lparam);
		}
	}
	break;

	case SPOTIFY_CLOSE_SINGLE_TRACK:
	{

		EnterCriticalSection(&user_data->spotify_lock);

		sp_session_player_play(user_data->spotify, false);

		track_ended(user_data);

		LeaveCriticalSection(&user_data->spotify_lock);
	}
	break;

	case SPOTIFY_DOWNLOAD_STATUS:
	{

		EnterCriticalSection(&user_data->spotify_lock);

		if (wparam == -1)
		{
			//errore di encoding
		}
		else if (wparam == 0)
		{
			//tick di download
			if (user_data->guiController)
			{
				char message[1001];
				memset(message, 0, 1001);

				float percent = user_data->actual_samples / (float)user_data->track_total_samples;
				sprintf_s(message, "Avanzamento download ... (%d\%)", percent);
				user_data->guiController->LogMessage(message);
			}
		}

		LeaveCriticalSection(&user_data->spotify_lock);
	}
	break;

	}
	return 0L;
}

DWORD _stdcall SpotifyMainProc(LPVOID data)
{
	SpotifyUserData *user_data = (SpotifyUserData *)data;

	if (user_data)
	{
		SpotifyMainLoop(user_data);
	}

	return (DWORD)-1;
}

void SpotifyMainLoop(SpotifyUserData *user_data)
{
	//avvio il loop
	while (user_data->spotify_active)
	{
		EnterCriticalSection(&user_data->spotify_lock);

		if (!user_data->spotify_timeout)
		{
			while (!user_data->spotify_flag)
			{
				SleepConditionVariableCS(&user_data->spotify_cond, &user_data->spotify_lock, INFINITE);
			}
		}
		else
		{
			SleepConditionVariableCS(&user_data->spotify_cond, &user_data->spotify_lock, user_data->spotify_timeout);
		}


		//guardia
		if (!user_data->spotify_active)
		{
			user_data->spotify_close_flag = 1;
			WakeAllConditionVariable(&user_data->spotify_close_cond);
			LeaveCriticalSection(&user_data->spotify_lock);

			return;
		}

		LeaveCriticalSection(&user_data->spotify_lock);

		do
		{
			sp_session_process_events(user_data->spotify, &user_data->spotify_timeout);
		} while (!user_data->spotify_timeout);
	}
}

sp_track *GetRandomTrack(sp_playlistcontainer *pc)
{
	sp_track *track = 0;

	if (pc)
	{
		if (sp_playlistcontainer_is_loaded(pc))
		{
			int num_p = sp_playlistcontainer_num_playlists(pc);

			srand(time(0));

			int des_playlist = rand() % num_p;

			for (int k = 0; k < num_p; k++)
			{
				sp_playlist *play = sp_playlistcontainer_playlist(pc, k);

				if (play && sp_playlist_is_loaded(play))
				{
					int num_t = sp_playlist_num_tracks(play);

					srand(time(0));

					int des_track = rand() % num_t;

					for (int j = 0; j < num_t; j++)
					{
						sp_track *tmp = sp_playlist_track(play, j);

						if (tmp && sp_track_is_loaded(tmp))
						{
							if (k == des_playlist && j == des_track)
							{
								track = tmp;
								break;
							}
						}
					}
				}
			}
		}
	}

	return track;
}

/////spotify callbacks

void __stdcall  logged_in(sp_session *session, sp_error error)
{
	SpotifyUserData *context = (SpotifyUserData *)sp_session_userdata(session);

	EnterCriticalSection(&context->spotify_lock);

	context->container = sp_session_playlistcontainer(session);
	sp_playlistcontainer_add_callbacks(context->container, context->container_cb, context->guiController);

	LeaveCriticalSection(&context->spotify_lock);

	PostMessage(context->spotify_window, SPOTIFY_LOGGED_IN, (WPARAM)error, 0);
}


void __stdcall logged_out(sp_session *session)
{
	SpotifyUserData *context = (SpotifyUserData *)sp_session_userdata(session);

	PostMessage(context->spotify_window, SPOTIFY_LOGGED_OUT, 0, 0);
}

void __stdcall  metadata_updated(sp_session *session)
{
	SpotifyUserData *context = (SpotifyUserData *)sp_session_userdata(session);


	PostMessage(context->spotify_window, SPOTIFY_METADATA_UPDATED, 0, 0);

}

void __stdcall  connection_error(sp_session *session, sp_error error)
{

}

void __stdcall message_to_user(sp_session *session, const char *message)
{

}

void __stdcall notify_main_thread(sp_session *session)
{
	SpotifyUserData *context = (SpotifyUserData *)sp_session_userdata(session);

	EnterCriticalSection(&context->spotify_lock);

	context->spotify_flag = 1;

	WakeAllConditionVariable(&context->spotify_cond);

	LeaveCriticalSection(&context->spotify_lock);
}

int PrepareEncoder(SpotifyUserData *context, const sp_audioformat *fmt)
{
	if (!context || !context->lame) return 0;

	//context->lame = lame_init();

	//if ( !context->lame ) return 0;

	lame_set_num_channels(context->lame, fmt->channels);
	lame_set_in_samplerate(context->lame, fmt->sample_rate); // 
	lame_set_brate(context->lame, 320); //320kbps
	lame_set_mode(context->lame, STEREO); //stereo
	lame_set_quality(context->lame, 2); //alta qualita

	context->track_total_samples = (int)ceil(((double)context->track_total_samples * fmt->sample_rate) / 1000.0);

	if (lame_init_params(context->lame) < 0)
	{
		lame_close(context->lame);
		context->lame = 0;
		return -1;
	}

	context->encoder_ready = 1;

	return 1;
}

int AllocMp3Buffer(SpotifyUserData *context, int num_frames)
{
	//if ( !context->lame ) return 0;


	//context->mp3_buffersize = 2 * num_frames + 7200; //non lo uso ..... mi tengo molto largo

	context->mp3_buffersize = 500000;

	context->encoder_buffer = malloc(context->mp3_buffersize);

	memset(context->encoder_buffer, 0, context->mp3_buffersize);


	return 1;
}

int FreeMP3Buffer(SpotifyUserData *context)
{
	if (context->encoder_buffer) free(context->encoder_buffer);

	context->encoder_buffer = 0;
	context->mp3_buffersize = 0;

	return 0;
}

int CloseEncoder(SpotifyUserData *context)
{
	if (!context) return 0;

	lame_close(context->lame);
	context->lame = 0;

	FreeMP3Buffer(context);
}


int __stdcall music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
{


	SpotifyUserData *context = (SpotifyUserData *)sp_session_userdata(session);

	int ret_frames = 0;

	if (context && !context->encoder_ready)
	{
		EnterCriticalSection(&context->spotify_buffer_lock);
		//preparo l'encoder
		if (!PrepareEncoder(context, format)) exit(1);
		//configuro il buffer
		//if ( !AllocMp3Buffer(context, num_frames) ) exit(1);

		//primo frame ritardo
		WakeAllConditionVariable(&context->spotify_buffer_cond);

		LeaveCriticalSection(&context->spotify_buffer_lock);

		return 0;
	}

	EnterCriticalSection(&context->spotify_lock);

	if (!context->track_downloaded)
	{
		if (context->actual_samples >= context->track_total_samples)
		{

			//printf("\r\nTraccia terminata (autoclose)!!\r\n");

			context->track_downloaded = 1;

			//sp_session_player_play(context->spotify, false);

			//track_ended(context);
			WakeAllConditionVariable(&context->spotify_cond);

			LeaveCriticalSection(&context->spotify_lock);

			PostMessage(context->spotify_window, SPOTIFY_CLOSE_SINGLE_TRACK, 0, 0);

			return 0;
		}
	}
	else
	{
		WakeAllConditionVariable(&context->spotify_cond);

		LeaveCriticalSection(&context->spotify_lock);

		//printf("Prossima traccia in caricamento....\r\n");

		return 0;
	}

	WakeAllConditionVariable(&context->spotify_cond);

	LeaveCriticalSection(&context->spotify_lock);




	EnterCriticalSection(&context->spotify_buffer_lock);

	//encoding, uso il worst case buffer lenght => 1.25 * n_sample + 7200

	int ret = lame_encode_buffer_interleaved(context->lame, (short *)frames, num_frames, (unsigned char *)context->encoder_buffer, 2 * num_frames + 7200);

	if (ret > 0)
	{
		//ok caricato il frame ed encodato, scrivo su file
		if (context->fp_mp3) fwrite(context->encoder_buffer, 1, ret, context->fp_mp3);

		ret_frames = num_frames; //ho comunque consumato tutto, buffering interno

		context->actual_samples += ret_frames;

		float secs = (float)context->actual_samples / format->sample_rate;
		float last_secs = (float)context->last_written_samples / format->sample_rate;

		//printf("Downloading Album[%i] | %s [%2.2f]\r\n", context->actual_download_album, context->actual_download_track_name, secs);
		if (secs - last_secs >= 10.0)
		{
			context->last_written_samples = context->actual_samples;
			//PostMessage(context->spotify_window, SPOTIFY_DOWNLOAD_STATUS, 0, 0);
			OutputDebugStringA("Avanzamento");
		}
	}
	else
	{

		if (!ret)
		{
			//printf("_");
			ret_frames = num_frames;
		}
		else
		{
			//printf("Encoder ERRROR[%d]\r\n", ret);
			//PostMessage(context->spotify_window, SPOTIFY_DOWNLOAD_STATUS, (WPARAM)-1, 0);
			OutputDebugStringA("Errore encoder");
			ret_frames = 0;

		}
	}

	WakeAllConditionVariable(&context->spotify_buffer_cond);

	LeaveCriticalSection(&context->spotify_buffer_lock);

	return ret_frames;
}

void __stdcall play_token_lost(sp_session *session)
{

}

void __stdcall log_message(sp_session *session, const char *data)
{
	//OutputDebugStringA(data);
	SpotifyUserData *context = (SpotifyUserData *)sp_session_userdata(session);

	if (context && data)
	{
		int len = strnlen(data, 2000);

		if (len > 0)
		{
			char *str_msg = (char *)malloc(len + 1);
			memset(str_msg, 0, len + 1);
			memcpy(str_msg, data, len);

			PostMessage(context->spotify_window, SPOTIFY_LOG_MESSAGE, 0, (LPARAM)str_msg);
		}
	}
}

void __stdcall end_of_track(sp_session *session)
{
	SpotifyUserData *context = (SpotifyUserData *)sp_session_userdata(session);

	//traccia terminata, vado avant

	track_ended(context);
}


void track_ended(SpotifyUserData *context)
{
	if (context && context->encoder_buffer && context->track)
	{

		//printf("\r\nTraccia %s terminata!!\r\n\r\n", context->actual_download_track_name);

		//flush dei buffer dell'encoder

		int ret = lame_encode_flush(context->lame, (unsigned char *)context->encoder_buffer, context->mp3_buffersize);

		if (ret > 0 && context->fp_mp3)
		{
			//scrivo l'ultimo frame

			fwrite(context->encoder_buffer, 1, ret, context->fp_mp3);
		}

		//chiudo l'encoder ed il file

		//CloseEncoder(context);

		if (context->fp_mp3) fclose(context->fp_mp3);

		context->fp_mp3 = 0;

		//azzero variabili di stato

		context->actual_samples = 0;
		context->track_downloaded = 0;

		context->actual_download_track_name = 0;

		//incremento indice di traccia e posto
		context->actual_download_track++;

		context->encoder_ready = 0;

		sp_session_player_unload(context->spotify);

		//notifica al thread main il termine delle operazioni

	}
}

void __stdcall streaming_error(sp_session *session, sp_error error)
{

}

void __stdcall userinfo_updated(sp_session *session)
{

}

void __stdcall start_playback(sp_session *session)
{

}

void __stdcall stop_playback(sp_session *session)
{

}

void __stdcall get_audio_buffer_stats(sp_session *session, sp_audio_buffer_stats *stats)
{

}

void __stdcall offline_status_updated(sp_session *session)
{

}

void __stdcall offline_error(sp_session *session, sp_error error)
{

}

void __stdcall credentials_blob_updated(sp_session *session, const char *blob)
{

}

void __stdcall connectionstate_updated(sp_session *session)
{

}

void __stdcall scrobble_error(sp_session *session, sp_error error)
{

}
