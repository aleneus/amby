#include <stdio.h>
#include <stdlib.h>

#include "beep.h"

int
main (int argc, char **argv)
{
  alsa_t a = {0};

  if (alsa_open_device(&a, "default", 44100, 10) != 0)
    {
      fprintf(stderr, "ALSA open failed\n");

      return EXIT_FAILURE;
    }

  alsa_play_beep(&a, 440, 100);
  alsa_play_beep(&a, 440*2, 100);
  alsa_play_beep(&a, 440*4, 100);

  alsa_close_device(&a);

  return EXIT_SUCCESS;
}
