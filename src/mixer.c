#include <sensors/sensors.h>

#include "conf.h"
#include "mixer.h"

int
mixer_init (mixer_t *mx, size_t capacity)
{
  mx->channels = calloc (capacity, sizeof (channel_t *));
  if (!mx->channels)
    return -1;

  mx->cap = capacity;
  mx->count = 0;
  pthread_mutex_init (&mx->m, NULL);

  mx->mix_buf = malloc (sizeof (float) * FRAMES_PER_PERIOD * CHANNELS_OUT);
  if (!mx->mix_buf)
    {
      free (mx->channels);
      return -1;
    }

  sensors_init (NULL);

  return 0;
}

void
mixer_free (mixer_t *mx)
{
  pthread_mutex_destroy (&mx->m);

  for (size_t i = 0; i < mx->count; ++i)
    free (mx->channels[i]);

  free (mx->channels);
  free (mx->mix_buf);
}

int
mixer_add_channel (mixer_t *mx, channel_t *ch)
{
  pthread_mutex_lock (&mx->m);

  if (mx->count >= mx->cap)
    {
      pthread_mutex_unlock (&mx->m);
      return -1;
    }

  mx->channels[mx->count++] = ch;
  pthread_mutex_unlock (&mx->m);

  return 0;
}

/* Render mix into provided float buffer. */
void
mixer_render_block (mixer_t *mx, float *out, snd_pcm_uframes_t frames)
{
  size_t samples = frames * CHANNELS_OUT;

  for (size_t i = 0; i < samples; ++i)
    out[i] = 0.0f;

  channel_t *local[MAX_LOCAL_CHANNELS];
  size_t n = 0;

  pthread_mutex_lock (&mx->m);

  n = (mx->count < MAX_LOCAL_CHANNELS) ? mx->count : MAX_LOCAL_CHANNELS;
  for (size_t i = 0; i < n; ++i)
    local[i] = mx->channels[i];

  pthread_mutex_unlock (&mx->m);

  for (size_t i = 0; i < n; ++i)
    channel_render_add (local[i], out, frames);
}

void
mixer_set_gain (mixer_t *mx, float gain)
{
  pthread_mutex_lock (&mx->m);

  for (size_t i = 0; i < mx->count; ++i)
    {
      atomic_store (&mx->channels[i]->gain, gain);
    }

  pthread_mutex_unlock (&mx->m);
}
