#include <signal.h>

#include "synth.h"
#include "channel.h"
#include "mixer.h"
#include "buff.h"
#include "threads.h"

buff_t shared;

int
main(int argc, char **argv)
{
  signal(SIGINT, interrupt);
  signal(SIGTERM, interrupt);

  shared.frames = FRAMES_PER_PERIOD;

  shared.buf = malloc(sizeof(float) * shared.frames * CHANNELS_OUT);
  if (!shared.buf)
    {
      perror("malloc");
      return 1;
    }

  pthread_mutex_init(&shared.m, NULL);
  pthread_cond_init(&shared.cv, NULL);
  shared.ready = 0;

  mixer_t mixer;

  if (mixer_init(&mixer, 16) < 0)
    {
      fprintf(stderr,"mixer init failed\n");
      return 1;
    }

  channel_t *ch = calloc(1, sizeof(channel_t));
  if (!ch)
    {
      fprintf(stderr,"channel allocation failed\n");
      return 1;
    }

  channel_init_sine(ch, 440.0, 0.2);
  mixer_add_channel(&mixer, ch);

  pthread_t prod_tid, cons_tid;

  if (pthread_create(&prod_tid, NULL, producer_thread, &mixer) != 0)
    {
      perror("pthread_create producer");
      return 1;
    }

  if (pthread_create(&cons_tid, NULL, consumer_thread, NULL) != 0)
    {
      perror("pthread_create consumer");
      keep_running = 0;
      pthread_join(prod_tid, NULL);

      return 1;
    }

  pthread_join(prod_tid, NULL);
  pthread_join(cons_tid, NULL);

  mixer_free(&mixer);
  pthread_mutex_destroy(&shared.m);
  pthread_cond_destroy(&shared.cv);

  free(shared.buf);

  return 0;
}
