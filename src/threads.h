#pragma once

#include "buff.h"
#include <math.h>

#define FORMAT SND_PCM_FORMAT_S16_LE
#define BUFFER_PERIODS 4

static volatile int keep_running = 1;

void interrupt (int signo);
void *producer_thread (void *arg);
ssize_t player_write_frames (snd_pcm_t *pcm, const short *buf,
                             snd_pcm_uframes_t frames);
void *consumer_thread (void *arg);
