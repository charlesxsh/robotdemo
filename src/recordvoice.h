#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include "msp_cmn.h"
#include "qivw.h"
#include "msp_errors.h"
#include "cJSON.h"



#define ALSA_PCM_NEW_HW_PARAMS_API
int record_to_file(const char *filename, int sec);
int play_from_file(const char *filename);

int text_to_voice(const char *text, char *session_id, const char *filename);
char *voice_to_text(char *session_id, const char *filename);
void parseAndplay(char *str, char *tv_session_id);