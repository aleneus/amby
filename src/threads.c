#include "mixer.h"
#include "threads.h"

void
interrupt(int signo)
{
  (void)signo;
  keep_running = 0;
}

void *
producer_thread(void *arg)
{
  mixer_t *mx = (mixer_t*)arg;

  extern buff_t shared;

  while (keep_running)
    {
      pthread_mutex_lock(&shared.m);
      while (shared.ready && keep_running)
        pthread_cond_wait(&shared.cv, &shared.m);

      if (!keep_running)
        {
          pthread_mutex_unlock(&shared.m);
          break;
        }

      mixer_render_block(mx, shared.buf, shared.frames);

      shared.ready = 1;
      pthread_cond_signal(&shared.cv);
      pthread_mutex_unlock(&shared.m);
    }

  return NULL;
}

ssize_t
player_write_frames(snd_pcm_t *pcm, const short *buf, snd_pcm_uframes_t frames)
{
  snd_pcm_sframes_t r = snd_pcm_writei(pcm, buf, frames);

  if (r < 0)
    r = snd_pcm_recover(pcm, r, 0);

  if (r < 0)
    return (ssize_t)r;

  return (ssize_t)r;
}

void *
consumer_thread(void *arg)
{
  (void)arg;

  int err;
  snd_pcm_t *pcm;
  snd_pcm_hw_params_t *hw;

  if ((err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
      fprintf(stderr, "snd_pcm_open error: %s\n", snd_strerror(err));
      keep_running = 0;

      return NULL;
    }

  snd_pcm_hw_params_malloc(&hw);
  snd_pcm_hw_params_any(pcm, hw);
  snd_pcm_hw_params_set_access(pcm, hw, SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(pcm, hw, FORMAT);
  snd_pcm_hw_params_set_channels(pcm, hw, CHANNELS_OUT);
  unsigned int rate = SAMPLE_RATE;
  snd_pcm_hw_params_set_rate_near(pcm, hw, &rate, 0);
  snd_pcm_uframes_t period_size = FRAMES_PER_PERIOD;
  snd_pcm_hw_params_set_period_size_near(pcm, hw, &period_size, 0);
  snd_pcm_uframes_t buffer_size = FRAMES_PER_PERIOD * BUFFER_PERIODS;
  snd_pcm_hw_params_set_buffer_size_near(pcm, hw, &buffer_size);

  if ((err = snd_pcm_hw_params(pcm, hw)) < 0)
    {
      fprintf(stderr, "snd_pcm_hw_params error: %s\n", snd_strerror(err));
      snd_pcm_close(pcm);
      snd_pcm_hw_params_free(hw);
      keep_running = 0;

      return NULL;
    }

  snd_pcm_hw_params_free(hw);

  if ((err = snd_pcm_prepare(pcm)) < 0)
    {
      fprintf(stderr, "snd_pcm_prepare error: %s\n", snd_strerror(err));
      snd_pcm_close(pcm);
      keep_running = 0;

      return NULL;
    }

  extern buff_t shared;

  short *outbuf = malloc(sizeof(short) * shared.frames * CHANNELS_OUT);
  if (!outbuf)
    {
      fprintf(stderr,"malloc outbuf failed\n");
      snd_pcm_close(pcm);

      return NULL;
    }

  while (keep_running)
    {
      pthread_mutex_lock(&shared.m);

      while (!shared.ready && keep_running)
        pthread_cond_wait(&shared.cv, &shared.m);

      if (!keep_running)
        {
          pthread_mutex_unlock(&shared.m);

          break;
        }

    snd_pcm_uframes_t frames = shared.frames;

    for (snd_pcm_uframes_t i = 0; i < frames * CHANNELS_OUT; ++i)
      {
        float v = shared.buf[i];

        if (v > 1.0) v = 1.0;
        if (v < -1.0) v = -1.0;

        outbuf[i] = (short)lrintf(v * 32767.0);
      }

    ssize_t wrote = player_write_frames(pcm, outbuf, frames);
    if (wrote < 0)
      {
        fprintf(stderr, "snd_pcm_writei failed: %s\n", snd_strerror((int)wrote));
        pthread_mutex_unlock(&shared.m);

        break;
      }

    else if ((snd_pcm_uframes_t)wrote < frames)
      {
        short *ptr = outbuf + wrote * CHANNELS_OUT;
        player_write_frames(pcm, ptr, frames - wrote);
      }

    shared.ready = 0;
    pthread_cond_signal(&shared.cv);
    pthread_mutex_unlock(&shared.m);
  }

  free(outbuf);
  snd_pcm_drain(pcm);
  snd_pcm_close(pcm);

  return NULL;
}
