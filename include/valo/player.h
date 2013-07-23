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


#ifndef _VL_PLAYER_H
#define _VL_PLAYER_H
#include <pthread.h>

typedef int64_t vl_time;
typedef struct VLTimer VLTimer;
typedef struct VLGL VLGL;

typedef struct VLImage {
  uint8_t *data;
  uint8_t *y;
  uint8_t *u;
  uint8_t *v;
  int width;
  int height;
  vl_time pts;
} VLImage;

typedef struct VLPlayer {
  VLGL *gl;
  char *url;
  VLTimer *timer;
  VLImage *image;
  pthread_t thread;
} VLPlayer;

VLPlayer *VLPlayer_construct(VLGL *gl, const char *url);

void VLPlayer_destroy(VLPlayer *player);

void VLPlayer_pause(VLPlayer *player);

void VLPlayer_seek(VLPlayer *player, vl_time time);


#endif
