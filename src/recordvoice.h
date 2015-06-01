#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

int record_to_file(const char *filename);
int play_from_file(const char *filename);

/* Function that run in new thread */
void thrd_listen_start(pthread_t *t, char *filename, pthread_mutex_t *mutex);
void *thrd_listen(void *arg);

void thrd_result_start(pthread_t *t, char *filename, pthread_mutex_t *mutex);
void *thrd_result(void *arg);