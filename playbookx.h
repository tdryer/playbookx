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

#ifndef _PLAYBOOKX_H_INCLUDED
#define _PLAYBOOKX_H_INCLUDED

// define opengl functions for both PlayBook and Linux
#ifdef plat_linux
	#define GL_GLEXT_PROTOTYPES
	#include <GL/glut.h>
#endif

#ifdef plat_playbook
	#include <bps/event.h>
	#include <EGL/egl.h>
	#include <GLES/gl.h>
#endif

#define EVENT_MOVE 0
#define EVENT_UP 1
#define EVENT_DOWN 2

// call this before any other playbookx function
int playbookx_init();

// set the callback for the update function
void set_update_function(void *func);

// set the callback for the render function
void set_render_function(void *func);

// set the callback for the event function
void set_event_function(void *func);

// call to start updating and rendering, returns on exit, call after the set_*
// functions
void main_loop();

// wrapper to prepend a filename with "app/native/" for playbook
char *asset_path(char *filename);

// wrapper for glOrtho/glOrthof in OpenGL/OpenGLES, since there's no neutral method
void orthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);

// internal functions
void playbookx_render();
void playbookx_update();
#ifdef plat_linux
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);
#endif
#ifdef plat_playbook
void playbook_touch_event(bps_event_t *event);
int playbook_events();
#endif

#endif
