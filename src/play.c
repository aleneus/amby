#include "play.h"

#include "buff.h"
#include "conf.h"
#include "threads.h"

ssize_t
player_write_frames (snd_pcm_t *pcm, const short *buf,
                     snd_pcm_uframes_t frames)
{
  snd_pcm_sframes_t r = snd_pcm_writei (pcm, buf, frames);

  if (r < 0)
    r = snd_pcm_recover (pcm, r, 0);

  if (r < 0)
    return (ssize_t)r;

  return (ssize_t)r;
}

void *
consumer_thread (void *arg)
{
  (void)arg;

  int err;
  snd_pcm_t *pcm;
  snd_pcm_hw_params_t *hw;

  if ((err = snd_pcm_open (&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
      fprintf (stderr, "snd_pcm_open error: %s\n", snd_strerror (err));
      keep_running = 0;

      return NULL;
    }

  snd_pcm_hw_params_malloc (&hw);
  snd_pcm_hw_params_any (pcm, hw);
  snd_pcm_hw_params_set_access (pcm, hw, SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format (pcm, hw, FORMAT);
  snd_pcm_hw_params_set_channels (pcm, hw, CHANNELS_OUT);
  unsigned int rate = SAMPLE_RATE;
  snd_pcm_hw_params_set_rate_near (pcm, hw, &rate, 0);
  snd_pcm_uframes_t period_size = FRAMES_PER_PERIOD;
  snd_pcm_hw_params_set_period_size_near (pcm, hw, &period_size, 0);
  snd_pcm_uframes_t buffer_size = FRAMES_PER_PERIOD * BUFFER_PERIODS;
  snd_pcm_hw_params_set_buffer_size_near (pcm, hw, &buffer_size);

  if ((err = snd_pcm_hw_params (pcm, hw)) < 0)
    {
      fprintf (stderr, "snd_pcm_hw_params error: %s\n", snd_strerror (err));
      snd_pcm_close (pcm);
      snd_pcm_hw_params_free (hw);
      keep_running = 0;

      return NULL;
    }

  snd_pcm_hw_params_free (hw);

  if ((err = snd_pcm_prepare (pcm)) < 0)
    {
      fprintf (stderr, "snd_pcm_prepare error: %s\n", snd_strerror (err));
      snd_pcm_close (pcm);
      keep_running = 0;

      return NULL;
    }

  extern buff_t shared;

  short *outbuf = buff_alloc_out_buffer (&shared);
  if (!outbuf)
    {
      fprintf (stderr, "malloc outbuf failed\n");
      snd_pcm_close (pcm);

      return NULL;
    }

  while (keep_running)
    {
      buff_lock_for_read (&shared);

      if (!keep_running)
        {
          buff_unlock (&shared);
          break;
        }

      snd_pcm_uframes_t frames = buff_fill_short_array (&shared, outbuf);

      ssize_t wrote = player_write_frames (pcm, outbuf, frames);
      if (wrote < 0)
        {
          fprintf (stderr, "snd_pcm_writei failed: %s\n",
                   snd_strerror ((int)wrote));
          buff_unlock (&shared);
          break;
        }

      else if ((snd_pcm_uframes_t)wrote < frames)
        {
          short *ptr = outbuf + wrote * CHANNELS_OUT;
          player_write_frames (pcm, ptr, frames - wrote);
        }

      buff_commit_read (&shared);
    }

  free (outbuf);
  snd_pcm_drain (pcm);
  snd_pcm_close (pcm);

  return NULL;
}
