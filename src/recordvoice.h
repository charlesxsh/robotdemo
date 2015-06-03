#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "msp_cmn.h"
#include "qivw.h"
#include "msp_errors.h"


#define ALSA_PCM_NEW_HW_PARAMS_API
#define TEMP_DIR 
int record_to_file(const char *filename);
int play_from_file(const char *filename);

int text_to_voice(const char *text, const char *params, const char *filename);
char *voice_to_text(const char *params, const char *filename);