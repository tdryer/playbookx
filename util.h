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

#ifndef _UTIL_H_INCLUDED
#define _UTIL_H_INCLUDED

// some helpful platform neutal functions

// next power of two
inline int nextp2(int x);

// load a png file into a texture
int load_texture(const char* filename, int* width, int* height, float* tex_x, float* tex_y, unsigned int *tex);

#endif
