#pragma once

#include "synth.h"
#include <pthread.h>
#include <stdatomic.h>

#define SAMPLE_RATE 44100

/* Channel abstraction. */
typedef struct
{
  synth_t synth;
  atomic_int enabled;
  _Atomic float gain;
} channel_t;

/* Initialize channel with a sine wave oscillator. */
void channel_init_sine (channel_t *ch, double freq, double amp);

/* Render channel audio and add it to the output buffer. */
void channel_render_add (channel_t *ch, float *out, size_t frames);
