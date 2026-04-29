#pragma once

#include <pthread.h>
#include <stdatomic.h>
#include "synth.h"

#define SAMPLE_RATE 44100

/* Channel abstraction */
typedef struct {
  synth_t synth;
  atomic_int enabled;
  _Atomic float gain;
} channel_t;

void channel_init_sine(channel_t *ch, double freq, double amp);
