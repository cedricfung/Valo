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


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include "valo/vlgl.h"
#include "valo/player.h"

static const struct timespec TIMER_TEN_MILLI = { 0, 1e7 };
static const vl_time TIMER_SEEK_NORMAL = -7;
static const vl_time TIMER_SEEK_RESET = -1;

struct VLTimer {
  bool abort;
  bool pause;
  vl_time base;
  vl_time duration;
  vl_time current;
  vl_time seek;
};

static void VLTimer_sync(VLTimer *timer, vl_time pts)
{
  if (timer->base == 0) {
    timer->base = av_gettime();
  }

  while (!timer->abort && timer->seek == TIMER_SEEK_NORMAL &&
      (timer->pause || av_gettime() - timer->base  < pts - 1e4)) {
    nanosleep(&TIMER_TEN_MILLI, NULL);
  }

  if (timer->seek == TIMER_SEEK_RESET) {
    timer->seek = TIMER_SEEK_NORMAL;
  }
  timer->base = av_gettime() - pts;
  timer->current = pts;
}

static int ffmpeg_interrupt_cb(void *opaque)
{
  VLTimer *timer = opaque;
  return timer && timer->abort;
}

static void *VLPlayer_thread(void *arg)
{
  VLPlayer *player = arg;
  VLImage *img = player->image;
  VLTimer *timer = player->timer;
  AVFormatContext *ic = NULL;
  AVCodecContext *vcc = NULL;
  AVStream *vs = NULL;
  AVCodec *vc = NULL;
  AVFrame *frame = NULL;
  AVFrame *_frame = NULL;
  AVPacket packet, *pkt = &packet;
  struct SwsContext *sws = NULL;
  int vi = -1, ret = 0;
  int got_frame;

  av_register_all();
  avformat_network_init();

  ic = avformat_alloc_context();
  ic->interrupt_callback.opaque = timer;
  ic->interrupt_callback.callback = ffmpeg_interrupt_cb;

  avformat_open_input(&ic, player->url, NULL, NULL);
  av_dump_format(ic, 0, player->url, 0);
  avformat_find_stream_info(ic, NULL);

  for (unsigned i = 0; i < ic->nb_streams; i++) {
    if (ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
      vs = ic->streams[i];
      vi = i;
      break;
    }
  }

  vcc = vs->codec;
  vc = avcodec_find_decoder(vcc->codec_id);
  avcodec_open2(vcc, vc, NULL);
  frame = avcodec_alloc_frame();

  timer->duration = ic->duration;

  while (!timer->abort) {
    if (timer->seek >= 0) {
      int64_t	target = av_rescale_q(timer->seek, AV_TIME_BASE_Q, vs->time_base);
      ret = avformat_seek_file(ic, vi, INT64_MIN, target, INT64_MAX, ~AVSEEK_FLAG_BYTE);
      if (ret < 0) {
        fprintf(stderr, "avformat_seek_file %d\n", ret);
      } else {
        avcodec_flush_buffers(vcc);
      }
      timer->seek = TIMER_SEEK_RESET;
    }

    if (av_read_frame(ic, pkt) >= 0) {
      if (pkt->stream_index == vi) {
        ret = avcodec_decode_video2(vcc, frame, &got_frame, pkt);
        if (ret < 0) {
          fprintf(stderr, "avcodec_decode_video2 %d\n", ret);
        } else if (got_frame) {
          if (img->width != frame->width || img->height != frame->height) {
            if (_frame) av_free(_frame);
            if (img->data) free(img->data);
            _frame = avcodec_alloc_frame();
            img->data = av_malloc(avpicture_get_size(PIX_FMT_YUV420P, frame->width, frame->height) * sizeof(uint8_t));
            avpicture_fill((AVPicture *)_frame, img->data, PIX_FMT_YUV420P, frame->width, frame->height);
            img->y = _frame->data[0];
            img->u = _frame->data[1];
            img->v = _frame->data[2];
            img->width = frame->width;
            img->height = frame->height;
            sws = sws_getCachedContext(sws, frame->width, frame->height, vcc->pix_fmt, frame->width, frame->height, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
          }
          sws_scale(sws, (const uint8_t * const*)frame->data, frame->linesize, 0, frame->height, _frame->data, _frame->linesize);

          img->pts = av_frame_get_best_effort_timestamp(frame);
          if (img->pts == AV_NOPTS_VALUE) {
            img->pts = 0;
          }
          img->pts = (img->pts - ic->start_time) * av_q2d(vs->time_base) * 1000000;
          VLTimer_sync(timer, img->pts);
        }
      }
      av_free_packet(pkt);
    } else {
      nanosleep(&TIMER_TEN_MILLI, NULL);
    }
  }

  av_free(_frame);
  av_free(frame);
  sws_freeContext(sws);
  avcodec_close(vcc);
  avformat_close_input(&ic);
  avformat_network_deinit();

  return 0;
}

VLPlayer *VLPlayer_construct(VLGL *gl, const char *url)
{
  VLPlayer *player = NULL;
  player = calloc(1, sizeof(VLPlayer));
  if (player == NULL) {
    fprintf(stderr, "[OOM: %d] VLPlayer_construct\n", __LINE__);
    return NULL;
  }

  player->gl = gl;
  player->url = strdup(url);
  player->image = calloc(1, sizeof(VLImage));
  player->timer = calloc(1, sizeof(VLTimer));

  pthread_create(&player->thread, NULL, VLPlayer_thread, player);

  return player;
}

void VLPlayer_destroy(VLPlayer *player)
{
  player->timer->abort = true;
  pthread_join(player->thread, NULL);
  free(player->url);
  free(player->image->data);
  free(player->image);
  free(player->timer);

  free(player);
}

void VLPlayer_pause(VLPlayer *player)
{
  VLTimer *timer = player->timer;

  timer->pause = !timer->pause;
}

void VLPlayer_seek(VLPlayer *player, vl_time time)
{
  VLTimer *timer = player->timer;

  time = timer->current + time;

  if (time < 0) {
    time = 0;
  } else if (time > timer->duration) {
    time = timer->duration;
  }

  timer->seek = time;
}
