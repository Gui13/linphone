/*
    liblinphone_tester - liblinphone test suite
    Copyright (C) 2013  Belledonne Communications SARL

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "liblinphone_tester.h"

static const char *_get_default_video_renderer(void){
#ifdef WIN32
	return "MSDrawDibDisplay";
#elif defined(ANDROID)
	return "MSAndroidDisplay";
#elif __APPLE__ && !defined(__ios)
	return "MSOSXGLDisplay";
#elif defined (HAVE_XV)
	return "MSX11Video";
#elif defined(HAVE_GL)
	return "MSGLXVideo";
#elif defined(__ios)
	return "IOSDisplay";
#else
	return "MSVideoOut";
#endif
}

static bool_t wait_for_eof(bool_t *eof, int *time,int time_refresh, int timeout) {
	while(*time < timeout && !*eof) {
		ms_usleep(time_refresh * 1000U);
		*time += time_refresh;
	}
	return *time < timeout;
}

static void eof_callback(LinphonePlayer *player, void *user_data) {
	bool_t *eof = (bool_t *)user_data;
	*eof = TRUE;
}

static void play_file(const char *filename, bool_t unsupported_format) {
	LinphoneCoreManager *lc_manager;
	LinphonePlayer *player;
	int res, time = 0;
	bool_t eof = FALSE;

	lc_manager = linphone_core_manager_new("marie_rc");
	CU_ASSERT_PTR_NOT_NULL(lc_manager);
	if(lc_manager == NULL) return;

	player = linphone_core_create_local_player(lc_manager->lc, ms_snd_card_manager_get_default_card(ms_snd_card_manager_get()), _get_default_video_renderer(), NULL);
	CU_ASSERT_PTR_NOT_NULL(player);
	if(player == NULL) goto fail;

	res = linphone_player_open(player, filename, eof_callback, &eof);
	if(unsupported_format) {
		CU_ASSERT_EQUAL(res, -1);
	} else {
		CU_ASSERT_EQUAL(res, 0);
	}
	if(res == -1) goto fail;

	CU_ASSERT_EQUAL((res = linphone_player_start(player)), 0);
	if(res == -1) goto fail;

	CU_ASSERT_TRUE(wait_for_eof(&eof, &time, 100, 13000));

	linphone_player_close(player);

	fail:
	if(player) linphone_player_destroy(player);
	if(lc_manager) linphone_core_manager_destroy(lc_manager);
}

static void playing_test(void) {
	play_file("sounds/hello_opus_h264.mkv", !linphone_local_player_matroska_supported());
}

test_t player_tests[] = {
	{	"Playing"	,	playing_test	}
};

test_suite_t player_test_suite = {
	"Player",
	NULL,
	NULL,
	sizeof(player_tests) / sizeof(test_t),
	player_tests
};
