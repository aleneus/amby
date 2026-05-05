#include <stdbool.h>

#include "conf.h"
#include "mixer.h"
#include "threads.h"

void
interrupt (int signo)
{
  (void)signo;
  keep_running = 0;
}
