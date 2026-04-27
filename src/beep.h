#ifndef BEEP_H
#define BEEP_H

#include <stdint.h>
#include <alsa/asoundlib.h>

/* ALSA PCM device handle and playback settings. */
typedef struct {
  snd_pcm_t *pcm;
  unsigned int rate;
  int16_t max_amp;
} alsa_t;

/* Open device. */
int alsa_open_device(alsa_t *a, const char *dev, unsigned int rate, unsigned int volume_percent);

/* Play sound. */
int alsa_play_beep(alsa_t *a, unsigned int freq_hz, unsigned int ms);

/* Close device. */
void alsa_close_device(alsa_t *a);

#endif /* ALSA_BEEP_H */
