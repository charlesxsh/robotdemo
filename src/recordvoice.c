#include "recordvoice.h"


int record_to_file(const char *filename){
	long loops;
	int rc, size, dir;
	int total_len = 0;
	unsigned int val;
	char *buffer;
	snd_pcm_t *rec_handle;
	snd_pcm_hw_params_t *rec_params;
	snd_pcm_uframes_t frames;

	FILE *fp_rec = fopen(filename, "wb");
	if(fp_rec == NULL){
		fprintf(stderr, "File creation failed.\n");
		return -1;
	}

	rc = snd_pcm_open(&rec_handle, "default", SND_PCM_STREAM_CAPTURE, 0);
	if(rc < 0){
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
		return -1;
	}

	snd_pcm_hw_params_alloca(&rec_params);
	snd_pcm_hw_params_any(rec_handle, rec_params);
	snd_pcm_hw_params_set_access(rec_handle, rec_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(rec_handle, rec_params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(rec_handle, rec_params, 1);
	val = 8000; //44100 bits per second sampling rate is cd quality
	snd_pcm_hw_params_set_rate_near(rec_handle, rec_params, &val, &dir);
	frames = 32;
	snd_pcm_hw_params_set_period_size_near(rec_handle, rec_params, &frames, &dir);
	rc = snd_pcm_hw_params(rec_handle, rec_params);
	if(rc < 0){
		fprintf(stderr, "unable to set parameters:%s\n", snd_strerror(rc));
		return -1;
	}

	snd_pcm_hw_params_get_period_size(rec_params, &frames, &dir);
	size = frames * 2;
	buffer = (char *)malloc(size);
	snd_pcm_hw_params_get_period_time(rec_params, &val, &dir);
	loops = 2000;
	while(loops-- > 0){
		rc = snd_pcm_readi(rec_handle, buffer, frames);

		if(rc == -EPIPE){
			fprintf(stderr, "overrun occured.\n");
			snd_pcm_prepare(rec_handle);
		}else if(rc < 0){
			fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
		}else if(rc != (int)frames){
			fprintf(stderr, "short read, read %d frames\n", rc);
		}

		rc = fwrite(buffer,1,size, fp_rec);
		total_len += size;
		printf("Recording...Total: %d bytes.\n", total_len);
		if (rc != size){
			fprintf(stderr, "short write: wrote %d bytes.\n", rc);
		}
	}
	snd_pcm_drain(rec_handle);
	snd_pcm_close(rec_handle);
	free(buffer);
	fclose(fp_rec);
	return 0;
}

int play_from_file(const char *filename){
	int rc,ret,size;
	unsigned int val;
	int dir = 0;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
	char *buffer;
	int channels = 1;
	int frequency = 8000;
	int bit = 16;
	int datablock = 2;
	FILE *fp = fopen(filename, "rb");

	rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if(rc < 0){
		fprintf(stderr, "Open pcm failed.\n");
		return -1;
	}
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(handle, params, channels);
	val = frequency;
	snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	rc = snd_pcm_hw_params(handle, params);
	if(rc < 0){
		fprintf(stderr, "set parameters to handle failed\n");
		return -1;
	}
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	size = frames * datablock;
	buffer = (char *)malloc(size);
	while(1){
		memset(buffer,0,sizeof(buffer));
		ret = fread(buffer,1,size,fp);
		if(ret == 0){
			printf("Write over!");
			break;
		}
		while((ret = snd_pcm_writei(handle, buffer, frames))<0){
			usleep(2000);
			if(ret == -EPIPE){
				fprintf(stderr, "underrun occurred\n");
				snd_pcm_prepare(handle);
			}else if(ret < 0){
				fprintf(stderr, "error from writei:%s\n", snd_strerror(ret));
			}
		}
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);
	return 0;
}

int text_to_voice(const char *text, const char *params, const char *filename){
	/* Prepare Part */
	const char *session_id = NULL;
	int ret = -1;
	unsigned int audio_len = 0;
	int synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
	FILE *fp = NULL;
	if(text == NULL || filename == NULL){
		fprintf(stderr, "Function text_to_voice parameters are invalid.\n");
		return -1;
	}
	/* Prepare part end */

	/* Session Begin */
	session_id = QTTSSessionBegin(params, &ret);
	if(ret != MSP_SUCCESS){
		fprintf(stderr, "QTTSSessionBegin failed. Error code %d.\n", ret);
		return -1;
	}
	ret = QTTSTextPut(session_id, text, strlen(text), NULL);
	if(ret != MSP_SUCCESS){
		fprintf(stderr, "QTTSTextPut failed. Error code %d.\n", ret);
		return -1;
	}
	fp = fopen(filename,"wb");
	if(fp == NULL){
		fprintf(stderr, "File %s created failed.\n", filename);
		return -1;
	}
	while(1){
		const void *data = QTTSAudioGet(session_id, &audio_len, &synth_status, &ret);
		if(data != NULL) fwrite(data, audio_len, 1, fp);
		if(synth_status == MSP_TTS_FLAG_DATA_END || ret != 0) break;
		sleep(1);
	}
	fclose(fp);
	ret = QTTSSessionEnd(session_id, NULL);
	if(ret != MSP_SUCCESS){
		fprintf(stderr, "QTTSSessionEnd failed. Error code %d.\n", ret);
	}
	return 0;
}

char *voice_to_text(const char *params, const char *filename){
	if(params == NULL || filename == NULL){
		fprintf(stderr, "Function voice_to_text parameters are invalid.\n");
		return NULL;
	}
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL){
		fprintf(stderr, "File %s read failed.\n", filename);
		return NULL;
	}
	int ret = -1;
	char buff_audiodata[5120];
	unsigned int audio_len = 0;
	int audio_status = 2;
	int ep_status = 0;
	int rec_status = 0;
	const char *session_id;

	session_id = QISRSessionBegin(NULL, params, &ret);
	if(ret != MSP_SUCCESS){
		fprintf(stderr, "QISRSessionBegin failed. Error code %d\n", ret);
		return NULL;
	}

	/* start to upload audio data */
	while(audio_status != MSP_AUDIO_SAMPLE_LAST){
		audio_len = fread(buff_audiodata, 1, 5120, fp);
		if (audio_len != 5120) audio_status = MSP_AUDIO_SAMPLE_LAST;
		ret = QISRAudioWrite(session_id, buff_audiodata, audio_len, audio_status, &ep_status, &rec_status);
		if (ret != 0){
			fprintf(stderr, "QISRAudioWrite failed. Error code %d.\n", ret);
			break;
		}else if (ep_status == MSP_EP_AFTER_SPEECH){
			fprintf(stderr, "End point of speech has been detected.\n");
			break;
		}
		usleep(160 * 1000); //160ms
	}

	const char *text_result;
	int rslt_result = 0;

	while(rslt_result != MSP_REC_STATUS_COMPLETE){
		text_result = QISRGetResult(session_id, &rslt_result, 5000, &ret);
		if(ret != 0){
			fprintf(stderr, "QISRGetResult failed. Error code %d.\n", ret);
			return NULL;
		}
		if(text_result != NULL){
			break;
		}
		//太累了懒得写了
		//Here need to fix the long text problem.
		usleep(200 * 1000);
	}
	int l = strlen(text_result);
	char *t = (char *)malloc(sizeof(char) * l + 1);
	strncpy(t, text_result, l);
	t[l] = '\0';
	ret = QISRSessionEnd(session_id, "normal end");
	if(ret != 0){
		fprintf(stderr, "QISRSessionEnd failed. Error code is %d.\n", ret);
	}
	
	return t;
}