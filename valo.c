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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include "valo/vlgl.h"
#include "valo/player.h"

static const vl_time TIMER_SEEK_STEP = 1e7;

#define VL_GLFW_CB
VL_GLFW_CB static void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  VLPlayer *player = glfwGetWindowUserPointer(window);
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_SPACE:
        VLPlayer_pause(player);
        VLGL_reset(player->gl);
        break;
      case GLFW_KEY_LEFT:
        VLGL_rotate(player->gl, 0, 0, 1, -10);
        break;
      case GLFW_KEY_RIGHT:
        VLGL_rotate(player->gl, 0, 0, 1, 10);
        break;
      case GLFW_KEY_UP:
        VLGL_rotate(player->gl, 1, 0, 0, -10);
        break;
      case GLFW_KEY_DOWN:
        VLGL_rotate(player->gl, 1, 0, 0, 10);
        break;
      case GLFW_KEY_B:
        VLPlayer_seek(player, -TIMER_SEEK_STEP);
        break;
      case GLFW_KEY_F:
        VLPlayer_seek(player, TIMER_SEEK_STEP);
        break;
      case GLFW_KEY_I:
        VLGL_zoom(player->gl, 0.1);
        break;
      case GLFW_KEY_O:
        VLGL_zoom(player->gl, -0.1);
        break;
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
      default:
        break;
    }
  }
}

VL_GLFW_CB static void framebuffer_size_cb(GLFWwindow* window, int w, int h)
{
  VLPlayer *player = glfwGetWindowUserPointer(window);
  glViewport(0, 0, w, h);
  VLGL_viewport(player->gl, w, h);
}

VL_GLFW_CB static void error_cb(int error, const char* description)
{
  fprintf(stderr, "%d, %s\n", error, description);
}

static int parse_poly_type(const char *type)
{
  if (!strcmp("cylinder", type)) {
    return POLY_CUBE;
  } else if (!strcmp("sphere", type)){
    return POLY_ICOSAHEDRON;
  } else {
    fprintf(stderr, "Invalid type, only 'cylinder' and 'sphere' supported.\n");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, const char *argv[])
{
  VLGL *gl = NULL;
  VLPlayer *player = NULL;
  GLFWwindow *window = NULL;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <panorama-type> <precision> <image-or-video>\n", argv[0]);
    return EXIT_FAILURE;
  }

  glfwSetErrorCallback(error_cb);
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  window = glfwCreateWindow(64, 64, argv[3], NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_cb);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_cb);

  VLGL_version();
  gl = VLGL_construct(parse_poly_type(argv[1]), atoi(argv[2]));
  player = VLPlayer_construct(gl, argv[3]);
  glfwSetWindowUserPointer(window, player);

  while (!glfwWindowShouldClose(window)) {
    VLGL_render(gl, player->image);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  VLGL_destroy(gl);
  VLPlayer_destroy(player);
  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
