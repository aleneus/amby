#pragma once

#include "channel.h"

/* Mixer. */
typedef struct
{
  channel_t **channels;
  size_t cap;
  size_t count;
  pthread_mutex_t m;
  float *mix_buf;
} mixer_t;

/* Initialize mixer with given capacity. */
int mixer_init (mixer_t *mx, size_t capacity);

/* Release mixer resources. */
void mixer_free (mixer_t *mx);

/* Add a new channel to the mixer. */
int mixer_add_channel (mixer_t *mx, channel_t *ch);

/* Mix all channels and render audio block. */
void mixer_render_block (mixer_t *mx, float *out, snd_pcm_uframes_t frames);

/* Set global gain for all mixer channels. */
void mixer_set_gain (mixer_t *mx, float gain);
