#include <alsa/asoundlib.h>
#include <stdio.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

int record_to_file(const char *filename);
int play_from_file(const char *filename);