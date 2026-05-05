#pragma once

/* Handle termination signals for graceful shutdown. */
void interrupt (int signo);

extern volatile int keep_running;
