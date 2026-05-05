#include <signal.h>
#include <stdbool.h>

#include "buff.h"
#include "channel.h"
#include "conf.h"
#include "mixer.h"
#include "play.h"
#include "producer.h"
#include "synth.h"
#include "threads.h"
#include "volume.h"

volatile int keep_running = 1;

buff_t shared;

int
add_sine_channel (mixer_t *mixer, double freq)
{
  channel_t *ch = calloc (1, sizeof (channel_t));
  if (!ch)
    {
      fprintf (stderr, "channel allocation failed\n");
      return -1;
    }

  channel_init_sine (ch, freq, 1);
  mixer_add_channel (mixer, ch);

  return 0;
}

int
main ()
{
  signal (SIGINT, interrupt);
  signal (SIGTERM, interrupt);

  if (buff_init (&shared, FRAMES_PER_PERIOD) != 0)
    {
      perror ("buff_init failed");
      return 1;
    }

  mixer_t mixer;

  if (mixer_init (&mixer, 16) < 0)
    {
      fprintf (stderr, "mixer init failed\n");
      return 1;
    }

  if (add_sine_channel (&mixer, 130.81) != 0)
    return 1;

  if (add_sine_channel (&mixer, 164.81) != 0)
    return 1;

  if (add_sine_channel (&mixer, 196.00) != 0)
    return 1;

  pthread_t prod_tid;
  pthread_t cons_tid;
  pthread_t ctrl_tid;

  if (pthread_create (&prod_tid, NULL, producer_thread, &mixer) != 0)
    {
      perror ("pthread_create producer");
      return 1;
    }

  if (pthread_create (&cons_tid, NULL, consumer_thread, NULL) != 0)
    {
      perror ("pthread_create consumer");
      keep_running = 0;
      pthread_join (prod_tid, NULL);

      return 1;
    }

  if (pthread_create (&ctrl_tid, NULL, volume_controller_thread, &mixer) != 0)
    {
      perror ("pthread_create volume controller");
      keep_running = 0;
      pthread_join (ctrl_tid, NULL);

      return 1;
    }

  pthread_join (prod_tid, NULL);
  pthread_join (cons_tid, NULL);
  pthread_join (ctrl_tid, NULL);

  mixer_free (&mixer);
  buff_destroy (&shared);

  return 0;
}
