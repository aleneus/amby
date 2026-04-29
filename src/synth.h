#pragma once

#include <alsa/asoundlib.h>

/* Synth interface. */
typedef struct synth_t
{
  double freq;
  double amp;
  double phase;
  double phase_inc;
  unsigned int sample_rate;

  void (*generate_block) (struct synth_t *s, float *out,
                          snd_pcm_uframes_t frames);
} synth_t;

void sine_generate_block (synth_t *s, float *out, snd_pcm_uframes_t frames);
void synth_init_sine (synth_t *s, double freq, double amp,
                      unsigned int sample_rate);
