/*
 * Copyright (c) 2012 Tom Dryer.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#ifdef plat_playbook
#include <assert.h>
#include <stdlib.h>
#include <screen/screen.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include "bbutil.h"
#endif

#include "playbookx.h"

// 60 fps limit
#define TIMER_DELAY 16.67

#ifdef plat_playbook
static screen_context_t screen_cxt;
#endif

void (*user_update)(float) = NULL;
void (*user_render)(void) = NULL;
void (*user_event)(char,int,int) = NULL;

struct timeval last_update_time;

int playbookx_init() {
	#ifdef plat_linux
		int argc = 1;
		char *arg = "playbookx";
		char ** argv = &arg;
		int width = 1024, height = 600;
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		glutInitWindowSize(width, height);
		glutCreateWindow(argv[0]);
	#endif
	#ifdef plat_playbook
		screen_create_context(&screen_cxt, 0);
		bps_initialize();
		// Use utility code to initialize EGL for rendering with GL ES 1.1
		if (EXIT_SUCCESS != bbutil_init_egl(screen_cxt)) {
			fprintf(stderr, "bbutil_init_egl failed\n");
			bbutil_terminate();
			screen_destroy_context(screen_cxt);
			return 0;
		}
		// Signal BPS library that navigator and screen events will be requested
		if (BPS_SUCCESS != screen_request_events(screen_cxt)) {
			fprintf(stderr, "screen_request_events failed\n");
			bbutil_terminate();
			screen_destroy_context(screen_cxt);
			return 0;
		}
		if (BPS_SUCCESS != navigator_request_events(0)) {
			fprintf(stderr, "navigator_request_events failed\n");
			bbutil_terminate();
			screen_destroy_context(screen_cxt);
			return 0;
		}
		// Signal BPS library that navigator orientation is not to be locked
		if (BPS_SUCCESS != navigator_rotation_lock(false)) {
			fprintf(stderr, "navigator_rotation_lock failed\n");
			bbutil_terminate();
			screen_destroy_context(screen_cxt);
			return 0;
		}
	#endif
	
	// initialize the timer
	gettimeofday(&last_update_time, NULL);

	return 0;
}

void set_update_function(void *func) {
	user_update = func;
}

void set_render_function(void *func) {
	user_render = func;
}

void set_event_function(void *func) {
	user_event = func;
}

void playbookx_render() {
	#ifdef plat_linux
		user_render();
		glutSwapBuffers();
	#endif
	#ifdef plat_playbook
		user_render();
		bbutil_swap();
	#endif
}

void playbookx_update() {
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	float elapsed_millis = ((current_time.tv_sec - last_update_time.tv_sec) 
				* 1000) + ((current_time.tv_usec - last_update_time.tv_usec)
				/ (float)1000);
	last_update_time = current_time;

	#ifdef plat_linux
		// set the next timer first
		glutTimerFunc(TIMER_DELAY, playbookx_update, 0);
		user_update(elapsed_millis);
		glutPostRedisplay();
	#endif
	#ifdef plat_playbook
		user_update(elapsed_millis);
		playbookx_render();
	#endif
}

#ifdef plat_linux
void glut_mouse(int button, int state, int x, int y) {
	if (state == GLUT_UP) {
		user_event(EVENT_UP, x, y);
	}
	else if (state == GLUT_DOWN) {
		user_event(EVENT_DOWN, x, y);
	}
}
#endif

#ifdef plat_linux
void glut_motion(int x, int y) {
	user_event(EVENT_MOVE, x, y);
}
#endif

#ifdef plat_playbook
void playbook_touch_event(bps_event_t *event) {
	screen_event_t screen_event = screen_event_get_event(event);
	int screen_val;
	screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE,
		&screen_val);
	int pair[2];
	screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_SOURCE_POSITION,
		pair);
	int id;
	screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TOUCH_ID,
		&id);
	// filter out all but the first finger touch
	if (id != 0) {
		return;
	}
	switch (screen_val) {
		case SCREEN_EVENT_MTOUCH_TOUCH:
			user_event(EVENT_DOWN, pair[0], pair[1]);
			break;
		case SCREEN_EVENT_MTOUCH_MOVE:
			user_event(EVENT_MOVE, pair[0], pair[1]);
			break;
		case SCREEN_EVENT_MTOUCH_RELEASE:
			user_event(EVENT_UP, pair[0], pair[1]);
			break;
	}
}
#endif

#ifdef plat_playbook
int playbook_events() {
	int rc;
	bps_event_t *event = NULL;
	unsigned int e_code;
	
	// get events until there are no more
	while(1) {
		rc = bps_get_event(&event, 0);
		assert(rc == BPS_SUCCESS);
		if (event) {
			e_code = bps_event_get_code(event);
			int domain = bps_event_get_domain(event);
			if (domain == screen_get_domain()) {
				playbook_touch_event(event);
			} else if (NAVIGATOR_EXIT == e_code) {
				return 1; // exit
			} else if (NAVIGATOR_WINDOW_INACTIVE == e_code) {
				// wait for activation or exit
				for (;;) {
					// block until event available
					rc = bps_get_event(&event, -1);
					assert(rc == BPS_SUCCESS);
					e_code = bps_event_get_code(event);
					// reactivate or exit nicely
					if (e_code == NAVIGATOR_WINDOW_ACTIVE) {
						break;
					} else if (NAVIGATOR_EXIT == e_code) {
						return 1; // exit
					}
				}
			}
		} else {
			return 0;
		}
	}
}
#endif

// *_function and init must have been called
void main_loop() {
	#ifdef plat_linux
		glutTimerFunc(0, playbookx_update, 0);
		glutDisplayFunc(playbookx_render);
		glutMotionFunc(glut_motion); // mouse move
		glutMouseFunc(glut_mouse); // mouse buttons
		glutMainLoop();
	#endif
	#ifdef plat_playbook
		while (!playbook_events()) {
			playbookx_update();
    	}
		screen_stop_events(screen_cxt);
		bps_shutdown();
		bbutil_terminate();
		screen_destroy_context(screen_cxt);
	#endif
}

char *asset_path(char *filename) {
	#ifdef plat_linux
		return filename;
	#endif
	#ifdef plat_playbook
		char *prefix = "app/native/";
		char *p = malloc(11 + strlen(filename) + 1);
		strcat(p, prefix);
		strcat(p, filename);
		return p;
	#endif
}

void orthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
	#ifdef plat_linux
		glOrtho(left, right, bottom, top, near, far);
	#endif
	#ifdef plat_playbook
		glOrthof(left, right, bottom, top, near, far);
	#endif
}


