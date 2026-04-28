#include "channel.h"

void
channel_init_sine(channel_t *ch, double freq, double amp)
{
  synth_init_sine(&ch->synth, freq, amp, SAMPLE_RATE);
  atomic_store(&ch->enabled, 1);
  atomic_store(&ch->gain, 1.0);
}
