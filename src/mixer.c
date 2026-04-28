#include "mixer.h"

int
mixer_init(mixer_t *mx, size_t capacity)
{
  mx->channels = calloc(capacity, sizeof(channel_t*));
  if (!mx->channels)
    return -1;

  mx->cap = capacity;
  mx->count = 0;
  pthread_mutex_init(&mx->m, NULL);

  mx->mix_buf = malloc(sizeof(float) * FRAMES_PER_PERIOD * CHANNELS_OUT);
  if (!mx->mix_buf)
    {
      free(mx->channels);
      return -1;
    }

  return 0;
}

void
mixer_free(mixer_t *mx)
{
  pthread_mutex_destroy(&mx->m);

  for (size_t i = 0; i < mx->count; ++i)
    free(mx->channels[i]);

  free(mx->channels);
  free(mx->mix_buf);
}

int
mixer_add_channel(mixer_t *mx, channel_t *ch)
{
  pthread_mutex_lock(&mx->m);

  if (mx->count >= mx->cap)
    {
      pthread_mutex_unlock(&mx->m);
      return -1;
    }

  mx->channels[mx->count++] = ch;
  pthread_mutex_unlock(&mx->m);

  return 0;
}

/* Render mix into provided float buffer. */
void
mixer_render_block(mixer_t *mx, float *out, snd_pcm_uframes_t frames)
{
  size_t samples = frames * CHANNELS_OUT;

  for (size_t i = 0; i < samples; ++i)
    out[i] = 0.0;

  pthread_mutex_lock(&mx->m);

  size_t n = mx->count;
  channel_t **local = malloc(sizeof(channel_t*) * n);

  if (local)
    for (size_t i = 0; i < n; ++i)
      local[i] = mx->channels[i];

  pthread_mutex_unlock(&mx->m);

  if (!local)
    return;

  float *temp = malloc(sizeof(float) * samples);
  if (!temp)
    {
      free(local);
      return;
    }

  for (size_t ci = 0; ci < n; ++ci)
    {
      channel_t *ch = local[ci];
      if (!atomic_load(&ch->enabled))
        continue;

      ch->synth.generate_block(&ch->synth, temp, frames);
      float gain = atomic_load(&ch->gain);

      for (size_t s = 0; s < samples; ++s)
        out[s] += temp[s] * gain;
    }

  free(temp);
  free(local);
}
