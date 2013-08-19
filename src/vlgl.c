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


#define GL_GLEXT_PROTOTYPES
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "3dm/3dm.h"
#include "3dm/poly.h"
#include "valo/vlgl.h"
#include "valo/player.h"

#define VLGL_CHECK_ERROR() do { \
  for (GLenum err = glGetError(); err != GL_NO_ERROR; err = glGetError()) { \
    fprintf(stderr, "[GL: %d]: %d, %s\n", __LINE__, err, gluErrorString(err)); \
  } \
} while(0)

#define VLGL_CHECK_STATUS(T,O,S) do { \
  GLint status; \
  glGet##T##iv(O, S, &status); \
  if (GL_FALSE == status) { \
    GLint len; \
    glGet##T##iv(O, GL_INFO_LOG_LENGTH, &len); \
    char *log = malloc(len * sizeof(char)); \
    glGet##T##InfoLog(O, len, NULL, log); \
    fprintf(stderr, "[GL: %d]: %d, %s\n", __LINE__, S, log); \
    free(log); \
  } \
} while (0)

#define VLGL_VERT_ID " \
#version 430 \n \
uniform mat4 u_model; \
uniform mat4 u_view; \
uniform mat4 u_proj; \
uniform mat4 u_tex; \
layout(location=7) in vec4 a_position; \
layout(location=3) in vec4 a_texcoord; \
smooth out vec2 v_texcoord; \
vec4 stereo(vec4 v) { \
  float len = length(v.xyz); \
  return vec4(v.x / (len - v.z), v.y / (len - v.z), 0, v.w); \
} \
void main() { \
  v_texcoord = (u_tex * a_texcoord).xy; \
  gl_Position = u_proj * u_view * stereo(u_model * a_position); \
} \
"

#define VLGL_FRAG_YUV " \
#version 430 \n \
uniform sampler2D tex_y; \
uniform sampler2D tex_u; \
uniform sampler2D tex_v; \
smooth in vec2 v_texcoord; \
smooth out vec4 color; \
void main() { \
  vec3 yuv, rgb; \
  yuv.x = texture2D(tex_y, v_texcoord).x; \
  yuv.y = texture2D(tex_u, v_texcoord).x - 0.5; \
  yuv.z = texture2D(tex_v, v_texcoord).x - 0.5; \
  rgb = mat3(1,1,1,0,-.34413,1.772,1.402,-.71414,0) * yuv; \
  color = vec4(rgb, 1.0); \
} \
"

static GLuint VLGL_create_shader(GLuint type, const char *source)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, (const GLchar **)&source, NULL);
  glCompileShader(shader);

  VLGL_CHECK_STATUS(Shader, shader, GL_COMPILE_STATUS);
  return shader;
}

static GLuint VLGL_create_program(GLuint vert, GLuint frag)
{
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glLinkProgram(prog);

  VLGL_CHECK_STATUS(Program, prog, GL_LINK_STATUS);
  glDetachShader(prog, vert);
  glDetachShader(prog, frag);
  return prog;
}

VLGL *VLGL_construct(enum poly_type type, int precision)
{
  VLGL *gl = NULL;
  GLuint vert, frag, prog;

  gl = calloc(1, sizeof(VLGL));
  if (gl == NULL) {
    fprintf(stderr, "[OOM: %d] VLGL_construct\n", __LINE__);
    return NULL;
  }
  gl->poly = poly_create(type, precision);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  vert = VLGL_create_shader(GL_VERTEX_SHADER, VLGL_VERT_ID);
  frag = VLGL_create_shader(GL_FRAGMENT_SHADER, VLGL_FRAG_YUV);
  prog = VLGL_create_program(vert, frag);
  glDeleteShader(vert);
  glDeleteShader(frag);

  gl->program = prog;
  gl->v_position = 7;
  gl->v_texcoord = 3;
  gl->u_model = glGetUniformLocation(prog, "u_model");
  gl->u_view = glGetUniformLocation(prog, "u_view");
  gl->u_proj = glGetUniformLocation(prog, "u_proj");
  gl->u_tex = glGetUniformLocation(prog, "u_tex");
  gl->samplers[0] = glGetUniformLocation(prog, "tex_y");
  gl->samplers[1] = glGetUniformLocation(prog, "tex_u");
  gl->samplers[2] = glGetUniformLocation(prog, "tex_v");

  glGenVertexArrays(1, &(gl->vao));
  glBindVertexArray(gl->vao);
  glGenBuffers(1, &(gl->vbo));
  glBindBuffer(GL_ARRAY_BUFFER, gl->vbo);
  glBufferData(GL_ARRAY_BUFFER, gl->poly->v_len * sizeof(float), gl->poly->vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(gl->v_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
  glEnableVertexAttribArray(gl->v_position);
  glGenBuffers(1, &(gl->tbo));
  glBindBuffer(GL_ARRAY_BUFFER, gl->tbo);
  glBufferData(GL_ARRAY_BUFFER, gl->poly->t_len * sizeof(float), gl->poly->texcoords, GL_STATIC_DRAW);
  glVertexAttribPointer(gl->v_texcoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
  glEnableVertexAttribArray(gl->v_texcoord);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenBuffers(1, &(gl->ebo));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, gl->poly->i_len * sizeof(float), gl->poly->indices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glGenTextures(3, gl->textures);
  for (int i = 0; i < 3; i++) {
    glBindTexture(GL_TEXTURE_2D, gl->textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  gl->vw = 1; gl->vh = 1; gl->vz = 1;
  gl->rotate_v = -90;
  gl->m_model = mat4d_rotate(mat4d_identity(), (vec4d)vector_new(1, 0, 0), -90);
  gl->m_proj = mat4d_ortho(45 * gl->vz, gl->vw / gl->vh, 1, 10);
  gl->m_view = mat4d_look_at((vec4d)vector_new(0, 0, -1), (vec4d)vector_new(0), (vec4d)vector_new(0,1,0));
  gl->m_tex = mat4d_identity();

  VLGL_CHECK_ERROR();
  return gl;
}

void VLGL_destroy(VLGL *gl)
{
  poly_destroy(gl->poly);
  glDeleteTextures(3, gl->textures);
  glDeleteBuffers(1, &(gl->tbo));
  glDeleteBuffers(1, &(gl->vbo));
  glDeleteBuffers(1, &(gl->ebo));
  glDeleteVertexArrays(1, &(gl->vao));
  glDeleteProgram(gl->program);
  VLGL_CHECK_ERROR();

  free(gl);
}

void VLGL_render(VLGL *gl, VLImage *img)
{
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(gl->program);

  if (img->y && img->u && img->v) {
    glActiveTexture(GL_TEXTURE0 + 64);
    glBindTexture(GL_TEXTURE_2D, gl->textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, img->width, img->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img->y);
    glUniform1i(gl->samplers[0], 64);

    glActiveTexture(GL_TEXTURE0 + 65);
    glBindTexture(GL_TEXTURE_2D, gl->textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (img->width+1)/2, (img->height+1)/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img->u);
    glUniform1i(gl->samplers[1], 65);

    glActiveTexture(GL_TEXTURE0 + 66);
    glBindTexture(GL_TEXTURE_2D, gl->textures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (img->width+1)/2, (img->height+1)/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img->v);
    glUniform1i(gl->samplers[2], 66);
  }

  glUniformMatrix4fv(gl->u_model, 1, GL_TRUE, mat4d_to_mat4f(gl->m_model).ptr);
  glUniformMatrix4fv(gl->u_view, 1, GL_TRUE, mat4d_to_mat4f(gl->m_view).ptr);
  glUniformMatrix4fv(gl->u_proj, 1, GL_TRUE, mat4d_to_mat4f(gl->m_proj).ptr);
  glUniformMatrix4fv(gl->u_tex, 1, GL_TRUE, mat4d_to_mat4f(gl->m_tex).ptr);

  glBindVertexArray(gl->vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl->ebo);

  glDrawElements(GL_TRIANGLES, gl->poly->i_len, GL_UNSIGNED_INT, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
  VLGL_CHECK_ERROR();
}

void VLGL_viewport(VLGL *gl, int w, int h)
{
  gl->vw = w; gl->vh = h;
  gl->m_proj = mat4d_ortho(45 * gl->vz, gl->vw / gl->vh, 1, 10);
}

void VLGL_rotate(VLGL *gl, double x, double y, double z, double degree)
{
  double delta = degree;
  if (x == 1 && y == 0 && z == 0) {
    if (gl->rotate_v + degree > 0) {
      delta = -gl->rotate_v;
      gl->rotate_v = 0;
    } else if (gl->rotate_v + degree < -180) {
      delta = -180 - gl->rotate_v;
      gl->rotate_v = -180;
    } else {
      gl->rotate_v += degree;
    }
  }
  gl->m_model = mat4d_rotate(gl->m_model, (vec4d)vector_new(x,y,z), delta);
}

void VLGL_zoom(VLGL *gl, double inc)
{
  gl->vz -= inc;
  if (gl->vz > 3.9) {
    gl->vz = 3.9;
  } else if (gl->vz < 0.1) {
    gl->vz = 0.1;
  }
  gl->m_proj = mat4d_ortho(45 * gl->vz, gl->vw / gl->vh, 1, 10);
}

void VLGL_reset(VLGL *gl)
{
  gl->vz = 1;
  gl->rotate_v = -90;
  gl->m_model = mat4d_rotate(mat4d_identity(), (vec4d)vector_new(1, 0, 0), -90);
  gl->m_proj = mat4d_ortho(45 * gl->vz, gl->vw / gl->vh, 1, 10);
}

void VLGL_version(void)
{
  int vs[2];
  glGetIntegerv(GL_MAJOR_VERSION, &vs[0]);
  glGetIntegerv(GL_MINOR_VERSION, &vs[1]);
  printf("OpenGL version: %s, %d, %d\n", glGetString(GL_VERSION), vs[0], vs[1]);
}
