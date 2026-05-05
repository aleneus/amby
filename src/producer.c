#include "producer.h"
#include "buff.h"
#include "mixer.h"
#include "threads.h"

void *
producer_thread (void *arg)
{
  mixer_t *mx = (mixer_t *)arg;

  extern buff_t shared;

  while (keep_running)
    {
      buff_lock_for_write (&shared);

      if (!keep_running)
        {
          buff_unlock (&shared);
          break;
        }

      buff_fill_from_mixer (&shared, mx);
      buff_commit_write (&shared);
    }

  return NULL;
}
