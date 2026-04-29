#pragma once

#include <stdbool.h>
#include <alsa/asoundlib.h>

#include "mixer.h"

/* Shared buffer between mixer and player. */
typedef struct {
  snd_pcm_uframes_t frames;
  float *samples;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  bool ready;
} buff_t;

/* Initialize buffer with storage for frames.  */
int buff_init (buff_t *b, snd_pcm_uframes_t frames);

/* Lock buffer for writing; blocks until buffer is consumed.  */
void buff_lock_for_write (buff_t *b);

/* Lock buffer for reading; blocks until data is ready.  */
void buff_lock_for_read (buff_t *b);

/* Release the mutex of buffer without changing its state.  */
void buff_unlock (buff_t *b);

/* Mark buffer as ready and signal the consumer.  */
void buff_commit_write (buff_t *b);

/* Mark buffer as empty and signal the producer.  */
void buff_commit_read (buff_t *b);

/* Fill buffer with samples rendered by mixer.  */
void buff_fill_from_mixer (buff_t *b, mixer_t *mx);

/* Convert buffer's float samples to array of shorts.  */
snd_pcm_uframes_t buff_fill_short_array (buff_t *b, short *out);

/* Deallocate resources held by buffer.  */
void buff_destroy (buff_t *b);

/* Allocate a short-integer array suitable for buffer's output.  */
short *buff_alloc_out_buffer (const buff_t *b);
