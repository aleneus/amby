#include "channel.h"
#include "conf.h"

void
channel_init_sine (channel_t *ch, double freq, double amp)
{
  synth_init_sine (&ch->synth, freq, amp, SAMPLE_RATE);
  atomic_store (&ch->enabled, 1);
  atomic_store (&ch->gain, 1.0);
}

void
channel_render_add (channel_t *ch, float *out, size_t frames)
{
  if (!atomic_load (&ch->enabled))
    return;

  float gain = atomic_load (&ch->gain);
  if (gain <= 0.0f)
    return;

  float temp[frames * CHANNELS_OUT];
  ch->synth.generate_block (&ch->synth, temp, frames);

  for (size_t i = 0; i < frames * CHANNELS_OUT; ++i)
    out[i] += temp[i] * gain;
}
