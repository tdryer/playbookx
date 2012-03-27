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
#include <math.h>
#include <stdlib.h>

#include "playbookx.h"
#include "util.h"

// touch circle
#define CIRCLE_SIZE 48
#define CIRCLE_VERTS 20
GLfloat* circle = NULL;
int touch_pos[2] = {0, 0};
char touch_down = 0;

// sliding cat
#define SLIDE_PERIOD 5000
float slide_position = 0, slide_time = 0;

// bouncing cat
#define BOUNCE_PERIOD 2000
#define BOUNCE_MIN 10
#define BOUNCE_MAX 600
float bounce_scale = 1, bounce_time = 0, bounce_alpha = 0;

// vertices and textures
GLuint ball_tex;
float ball_tex_xy[2];
GLfloat ball_verts[8] = {0, 0, 1, 0, 0, 1, 1, 1};
GLfloat ball_tex_coords[8];

GLfloat* create_circle(int x, int y, int radius, int verts) {
	float angle;
	GLfloat* new_circle = (GLfloat*) malloc(verts * 2 * sizeof(GLfloat));
	int i;
	for (i = 0; i < verts; i++) {
		angle = i*2*3.14159/verts;
		new_circle[i*2] = x + (cos(angle) * radius);
		new_circle[i*2+1] = y + (sin(angle) * radius);
	}
	return new_circle;
}

// called once per frame with time elapsed since last frame
void update(float elapsed_secs) {	
	slide_time = slide_time + elapsed_secs;
	slide_time = slide_time > SLIDE_PERIOD ? 0 : slide_time;
	slide_position = slide_time/SLIDE_PERIOD * 1024;
	
	bounce_time = bounce_time + elapsed_secs;
	bounce_time = bounce_time > BOUNCE_PERIOD ? 0 : bounce_time;
	float scale = ((1 - pow((bounce_time / BOUNCE_PERIOD)*2-1, 2)));
	bounce_scale = scale * (BOUNCE_MAX - BOUNCE_MIN) + BOUNCE_MIN;
	bounce_alpha = scale;
}

// called to render frame
void render() {
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, ball_verts);
	glBindTexture(GL_TEXTURE_2D, ball_tex);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glTexCoordPointer(2, GL_FLOAT, 0, ball_tex_coords);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// draw sliding cat
	glPushMatrix();
	glTranslatef(slide_position, 0, 0.0f);
	glScalef(128, 128, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();
	
	// draw bouncing cat
	glPushMatrix();
	glColor4f(1, 1, 1, bounce_alpha);
	glTranslatef((1024/2) - (bounce_scale/2), (600/2) - (bounce_scale/2), 0);
	glScalef(bounce_scale, bounce_scale, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	// draw the mouse/touch position
	if (touch_down) {
		glVertexPointer(2, GL_FLOAT, 0, circle);
		glPushMatrix();
		glTranslatef(touch_pos[0], touch_pos[1], 0.0f);
		glColor4f(1, 1, 1, 0.5);
		glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_VERTS);
	
		glPopMatrix();
	}
}

// called when there is a mouse/touch event
void event(char event_type, int x, int y) {
	if (event_type == EVENT_DOWN) {
		touch_down = 1;
	}
	else if (event_type == EVENT_UP) {
		touch_down = 0;
	}
	touch_pos[0] = x;
	touch_pos[1] = y;
}

// entry point for program
int main(int argc, char** argv) {
	// initialize playbookx
	playbookx_init();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glEnableClientState(GL_VERTEX_ARRAY);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	circle = create_circle(0, 0, CIRCLE_SIZE, CIRCLE_VERTS);

	if (load_texture(asset_path("icon.png"), NULL, NULL, &ball_tex_xy[0], 
				&ball_tex_xy[1], &ball_tex) != EXIT_SUCCESS) {
		fprintf(stderr, "Unable to load texture\n");
	}

	// texture coordinates
	ball_tex_coords[0] = 0;					ball_tex_coords[1] = ball_tex_xy[1];
	ball_tex_coords[2] = ball_tex_xy[0];	ball_tex_coords[3] = ball_tex_xy[1];
	ball_tex_coords[4] = 0;					ball_tex_coords[5] = 0;
	ball_tex_coords[6] = ball_tex_xy[0];	ball_tex_coords[7] = 0;

	int width = 1024, height = 600;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	orthof(0.0, width, height, 0.0, -1.0f, 1.0f);

	// set the callbacks for playbookx
	set_update_function(update);
	set_render_function(render);
	set_event_function(event);
	// enter the playbookx main loop
	main_loop();
	
	return 0;
}
