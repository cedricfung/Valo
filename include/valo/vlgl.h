/**
 * Valo - a panoramic image and video viewer
 *
 * Copyright (C) 2013 Cedric Fung <cedric@vec.io>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the Cedric Fung nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY Cedric Fung "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Cedric Fung BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _VL_GL_H
#define _VL_GL_H
#include <GL/gl.h>
#include <GL/glu.h>
#include "3dm/3dm.h"
#include "3dm/poly.h"

typedef struct VLImage VLImage;

typedef struct VLGL {
  GLuint program;
  GLuint vbo;
  GLuint tbo;
  GLuint ebo;
  GLuint vao;
  GLuint v_position;
  GLuint v_texcoord;
  GLuint u_model;
  GLuint u_view;
  GLuint u_proj;
  GLuint u_tex;
  GLuint textures[3];
  GLuint samplers[3];
  mat4d m_model;
  mat4d m_view;
  mat4d m_proj;
  mat4d m_tex;
  poly_t *poly;
  GLfloat vw, vh, vz;
} VLGL;

VLGL *VLGL_construct(enum poly_type type, int precision);

void VLGL_destroy(VLGL *gl);

void VLGL_render(VLGL *gl, struct VLImage *img);

void VLGL_viewport(VLGL *gl, int w, int h);

void VLGL_rotate(VLGL *gl, double x, double y, double z, double degree);

void VLGL_zoom(VLGL *gl, double inc);

void VLGL_reset(VLGL *gl);

void VLGL_version(void);

#endif
