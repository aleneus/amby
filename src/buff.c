#include <pthread.h>
#include <math.h>

#include "buff.h"
#include "conf.h"

int
buff_init (buff_t *b, snd_pcm_uframes_t frames)
{
  b->frames = frames;
  b->samples = malloc (sizeof(float) * b->frames * CHANNELS_OUT);

  if (!b->samples)
    return -1;

  pthread_mutex_init (&b->mutex, NULL);
  pthread_cond_init (&b->cond, NULL);
  b->ready = false;

  return 0;
}

void
buff_lock_for_write (buff_t *b)
{
  pthread_mutex_lock (&b->mutex);
  while (b->ready)
    pthread_cond_wait (&b->cond, &b->mutex);
}

void buff_commit_write (buff_t *b)
{
  b->ready = true;
  pthread_cond_signal (&b->cond);
  pthread_mutex_unlock (&b->mutex);
}

void
buff_unlock (buff_t *b)
{
  pthread_mutex_unlock (&b->mutex);
}

void
buff_lock_for_read (buff_t *b)
{
  pthread_mutex_lock(&b->mutex);
  while (!b->ready)
    pthread_cond_wait(&b->cond, &b->mutex);
}

void
buff_commit_read (buff_t *b)
{
  b->ready = false;
  pthread_cond_signal(&b->cond);
  pthread_mutex_unlock(&b->mutex);
}

void
buff_fill_from_mixer (buff_t *b, mixer_t *mx)
{
  mixer_render_block(mx, b->samples, b->frames);
}

snd_pcm_uframes_t
buff_fill_short_array (buff_t *b, short *out)
{
  size_t total_samples = (size_t)b->frames * CHANNELS_OUT;

  for (size_t i = 0; i < total_samples; ++i)
    {
      float v = b->samples[i];

      if (v > 1.0f) v = 1.0f;
      else if (v < -1.0f) v = -1.0f;

      out[i] = (short)lrintf (v * 32767.0f);
    }

  return b->frames;
}

void
buff_destroy (buff_t *b)
{
  if (!b) return;

  pthread_mutex_destroy (&b->mutex);
  pthread_cond_destroy (&b->cond);

  if (b->samples)
    {
      free (b->samples);
      b->samples = NULL;
    }
}

short *
buff_alloc_out_buffer (const buff_t *b)
{
  return (short *) malloc (sizeof (short) * b->frames * CHANNELS_OUT);
}
