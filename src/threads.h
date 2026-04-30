#pragma once

#include "buff.h"
#include <math.h>

/* Preferred PCM sample format. */
#define FORMAT SND_PCM_FORMAT_S16_LE

/* Number of periods in the hardware buffer. */
#define BUFFER_PERIODS 4

/* Handle termination signals for graceful shutdown. */
void interrupt (int signo);

/* Audio data generation. */
void *producer_thread (void *arg);

/* Audio processing and playback consumption thread. */
void *consumer_thread (void *arg);

/* Thermal-based gain adjustment thread. */
void *volume_controller_thread (void *arg);

/* Send PCM frames to the audio device. */
ssize_t player_write_frames (snd_pcm_t *pcm, const short *buf,
                             snd_pcm_uframes_t frames);

static volatile int keep_running = 1;
