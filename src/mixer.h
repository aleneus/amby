#pragma once

#include "channel.h"


/* Mixer. */
typedef struct {
  channel_t **channels;
  size_t cap;
  size_t count;
  pthread_mutex_t m;
  float *mix_buf;
} mixer_t;

int mixer_init(mixer_t *mx, size_t capacity);
void mixer_free(mixer_t *mx);
int mixer_add_channel(mixer_t *mx, channel_t *ch);
void mixer_render_block(mixer_t *mx, float *out, snd_pcm_uframes_t frames);
