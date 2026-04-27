#include <stdlib.h>
#include <errno.h>

#include "beep.h"

int alsa_open_device(alsa_t *a, const char *dev, unsigned int rate, unsigned int volume_percent)
{
  snd_pcm_hw_params_t *params;
  int err;

  a->rate = rate;
  a->max_amp = (volume_percent>100?100:volume_percent) * 32767 / 100;

  if ((err = snd_pcm_open(&a->pcm, dev, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    return err;

  snd_pcm_hw_params_malloc(&params);
  snd_pcm_hw_params_any(a->pcm, params);
  snd_pcm_hw_params_set_access(a->pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(a->pcm, params, SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_channels(a->pcm, params, 1);
  snd_pcm_hw_params_set_rate_near(a->pcm, params, &a->rate, 0);
  snd_pcm_hw_params(a->pcm, params);
  snd_pcm_hw_params_free(params);
  snd_pcm_prepare(a->pcm);

  return 0;
}

int alsa_play_beep(alsa_t *a, unsigned int freq_hz, unsigned int ms)
{
  unsigned int total = a->rate * ms / 1000;

  int16_t *buf = malloc(total * sizeof(int16_t));
  if (!buf)
    return -1;

  unsigned int spp = a->rate / freq_hz;

  if (spp == 0)
    spp = 1;

  for (unsigned int i = 0; i < total; ++i)
    buf[i] = ((i % spp) < (spp/2)) ? a->max_amp : -a->max_amp;

  unsigned int pos = 0;
  while (pos < total)
    {
      snd_pcm_sframes_t written = snd_pcm_writei(a->pcm, buf + pos, total - pos);

      if (written == -EPIPE)
        snd_pcm_prepare(a->pcm);

      else if (written < 0)
        {
          free(buf);
          return (int)written;
        }

      else pos += written;
    }

  free(buf);

  return 0;
}

void alsa_close_device(alsa_t *a)
{
  if (a->pcm)
    {
      snd_pcm_drain(a->pcm);
      snd_pcm_close(a->pcm);
      a->pcm = NULL;
    }
}
