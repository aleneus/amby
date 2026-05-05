#pragma once

/* The number of audio frames processed in a single period (block).
   This determines the granularity of the audio engine and the minimum
   latency. A frame consists of one sample per channel. */
#define FRAMES_PER_PERIOD 1024

/* The number of output audio channels. For example, 1 for mono, 2 for
   stereo. This value is used to calculate buffer sizes and to
   configure the hardware PCM parameters. */
#define CHANNELS_OUT 2

/* The maximum number of channels that can be processed by the mixer
   during a single render cycle.  This limit allows the use of a
   fixed-size stack buffer to avoid dynamic memory allocation in the
   real-time audio thread. */
#define MAX_LOCAL_CHANNELS 64

/* Preferred PCM sample format. */
#define FORMAT SND_PCM_FORMAT_S16_LE

/* Number of periods in the hardware buffer. */
#define BUFFER_PERIODS 4
