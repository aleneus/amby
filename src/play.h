#pragma once

#include <alsa/asoundlib.h>

/* Audio processing and playback consumption thread. */
void *consumer_thread (void *arg);

/* Send PCM frames to the audio device. */
ssize_t player_write_frames (snd_pcm_t *pcm, const short *buf,
                             snd_pcm_uframes_t frames);
