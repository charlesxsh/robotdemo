#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "msp_cmn.h"
#include "qivw.h"
#include "msp_errors.h"
#include "recordvoice.h"

#define BUFFER_FILE_DIR "buffer.raw"
int main(int argc, char *argv[]){
	pthread_t listen, result;
	pthread_mutex_t mutex; //used from lock listen and result
	pthread_mutex_init(&mutex, NULL);
	thrd_listen_start(&listen, BUFFER_FILE_DIR, &mutex);
	thrd_result_start(&result, BUFFER_FILE_DIR, &mutex);
	return 0;
}
