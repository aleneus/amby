#pragma once

#include <alsa/asoundlib.h>

/* Shared buffer between mixer and player. */
typedef struct {
  float *buf;
  snd_pcm_uframes_t frames;
  pthread_mutex_t m;
  pthread_cond_t cv;
  int ready;
} buff_t;
