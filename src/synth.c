#include <math.h>
#include <alsa/asoundlib.h>

#include "synth.h"

void
sine_generate_block(synth_t *s, float *out, snd_pcm_uframes_t frames)
{
  for (snd_pcm_uframes_t i = 0; i < frames; ++i)
    {
      float v = (float)(sin(s->phase) * s->amp);
      s->phase += s->phase_inc;
      if (s->phase > 2.0*M_PI)
        s->phase -= 2.0*M_PI;

      out[2*i] = v;
      out[2*i+1] = v;
    }
}

void
synth_init_sine(synth_t *s, double freq, double amp, unsigned int sample_rate)
{
  s->freq = freq;
  s->amp = amp;
  s->phase = 0.0;
  s->sample_rate = sample_rate;
  s->phase_inc = 2.0 * M_PI * s->freq / (double)s->sample_rate;
  s->generate_block = sine_generate_block;
}
