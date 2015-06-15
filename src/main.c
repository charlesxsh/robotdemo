#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include "msp_cmn.h"
#include "qivw.h"
#include "msp_errors.h"
#include "VAD.h"
//登录信息
#define USR "charlesxsh@163.com"
#define PWD "Xsh952644081"
#define LG_PARAM "appid = 55657997"

//只是识别
#define VT_PARAMS "engine_type=cloud,domain=iat,aue=raw;-1,auf=audio/L16;rate=8000,vad_timeout=1000,vad_speech_tail=1000,asr_ptt=false,result_type=plain,result_encoding=utf-8"

//识别语义
#define NL_PARAMS "engine_type=cloud,domain=iat,asr_sch=1,aue=speex-wb;7,auf=audio/L16;rate=16000,vad_timeout=1000,vad_speech_tail=1000,asr_ptt=false,result_type=json,result_encoding=utf-8,nlp_version=2.0"

//语音合成
#define TV_PARAMS "vcn=donaldduck,spd=80,text_encoding=UTF8,aue=speex-wb;7,ttp=text,vol=50,background_sound=0,sample_rate=16000"

//录音文件名
#define REC_FILE_NAME "rec_temp"
#define ALSA_PCM_NEW_HW_PARAMS_API

int main(int argc, char *argv[]){
	
    if(argc > 1 && atoi(argv[1])==1){
		play_from_file("1101391.raw");
		return 0;
	}

	int ret; //函数返回值，一般小于0就报错

	/*--------------------------------------------------录音设备准备和设置--------------------------------- */

	snd_pcm_t *rec_handle; //录音设备控制结构
	snd_pcm_hw_params_t *rec_params; //录音设备控制参数结构
	snd_pcm_uframes_t frames = 32; //录音每秒钟帧数
	int frequency = 16000; //采样频率16K
	int dir;
	ret = snd_pcm_open(&rec_handle, "default", SND_PCM_STREAM_CAPTURE, 0); //打开录音设备并设置为录音模式
	if (ret < 0) {
		fprintf(stderr, "Unable to open pcm device: %s\n", snd_strerror(ret));
		return -1;
	}
	snd_pcm_hw_params_alloca(&rec_params); //为参数结构体申请空间
	snd_pcm_hw_params_any(rec_handle, rec_params); //默认分配参数
	snd_pcm_hw_params_set_access(rec_handle, rec_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(rec_handle, rec_params, SND_PCM_FORMAT_S16_LE); //16位采样
	snd_pcm_hw_params_set_channels(rec_handle, rec_params, 1); //单通道
	snd_pcm_hw_params_set_rate_near(rec_handle, rec_params, &frequency, &dir);
	snd_pcm_hw_params_set_period_size_near(rec_handle, rec_params, &frames, &dir);
	snd_pcm_hw_params_get_period_size(rec_params, &frames, &dir);
	snd_pcm_hw_params_get_period_time(rec_params, &frequency, &dir);
	ret = snd_pcm_hw_params(rec_handle, rec_params);
	
	if (ret < 0) {
		fprintf(stderr, "unable to set parameters:%s\n", snd_strerror(ret));
		return -1;
	}

	/* -----------------------------------------云端服务准备---------------------------------------------- */
	ret = MSPLogin(USR, PWD, LG_PARAM);
	if(ret != MSP_SUCCESS){
		printf("MSPLogin failed, error code is: %d\n", ret);
		exit(1);
	}

    char *vt_session_id = NULL;
    char *tv_session_id = NULL;

    vt_session_id = QISRSessionBegin(NULL, NL_PARAMS, &ret);
    if(ret != MSP_SUCCESS){
        fprintf(stderr, "QISRSessionBegin failed. Error code %d\n", ret);
        return NULL;
    }

    tv_session_id = QTTSSessionBegin(TV_PARAMS, &ret);
    if(ret != MSP_SUCCESS){
        fprintf(stderr, "QTTSSessionBegin failed. Error code %d.\n", ret);
        return -1;
    }
	printf("已完成所有准备工作\n");


	/*------------------------------主循环，录音，识别，得到结果合成---------------------------------------*/
	int audio_status; //音频数据状态，2表示还有后续数据
	unsigned int audio_len = frames * 2; //音频长度
	int ep_status = 0; //返回状态
	int rec_status = 0; //音频数据状态
	const char *text_result; //识别结果指针
	int rslt_result = 0; //识别结果状态
	char buff[audio_len]; //音频缓冲区
	int vad_flag; //检测到说话为0
	_vad_reset();
	//struct timeval tv_begin;
	//struct timeval tv_end;
	//struct timezone tz;
	//主循环，录音到缓存，上传分析，解析结果，上传结果，播放
    while(1){
		//gettimeofday(&tv_begin, &tz);
		//等待用户说话
		printf("---->Waiting\n");
		vad_flag = 1;
		while(vad_flag == 1){
			snd_pcm_readi(rec_handle, buff, frames);
			vad_flag = vad_detect(buff, audio_len);
			printf("%d\n", vad_flag);
		}
		_vad_reset();

		//开始录音并上传语音直到不说话
		printf("---->Recording\n");
		audio_status = 2; //表示还有后续
		while (audio_status != MSP_AUDIO_SAMPLE_LAST) {	
			ret = snd_pcm_readi(rec_handle, buff, frames); //从录音设备文件读取音频数据放到buff里面
			if (ret == -EPIPE) {
				fprintf(stderr, "overrun occured.\n");
				snd_pcm_prepare(rec_handle);
			}
			else if (ret < 0) {
				fprintf(stderr, "error from read: %s\n", snd_strerror(ret));
			}
			else if (ret != (int)frames) {
				fprintf(stderr, "short read, read %d frames\n", ret);
			}
			ret = QISRAudioWrite(vt_session_id, buff, audio_len, audio_status, &ep_status, &rec_status);
			if (ret != 0) {
				fprintf(stderr, "QISRAudioWrite failed. Error code %d.\n", ret);
				break;
			}
			else if (ep_status == MSP_EP_AFTER_SPEECH) {
				audio_status = MSP_AUDIO_SAMPLE_LAST;
				fprintf(stderr, "End point of speech has been detected.\n");
				break;
			}
			//gettimeofday(&tv_end, &tz);
			//if (tv_end.tv_sec - tv_begin.tv_sec > 5) {
			//	break;
			//}
		}

		//得到结果
		int request_count = 0; //cannot exceed 5
		while (rslt_result != MSP_REC_STATUS_COMPLETE) {
			if(request_count >= 10){
				break;
			}
			request_count++;
			printf("getting...\n");
			text_result = QISRGetResult(vt_session_id, &rslt_result, 5000, &ret);
			if (ret != 0) {
				fprintf(stderr, "QISRGetResult failed. Error code %d.\n", ret);
				return NULL;
			}
			if (text_result != NULL) {
				printf("%s\n", text_result);
				break;
			}
			usleep(200 * 1000);
		}

		if(text_result == NULL){
			continue;
		}else{
			//parseAndplay(text_result, tv_session_id);
		}
	
    }

    //结束部分，释放资源
    ret = QISRSessionEnd(vt_session_id, "normal end");
    if(ret != 0){
        fprintf(stderr, "QISRSessionEnd failed. Error code is %d.\n", ret);
    }

    ret = QTTSSessionEnd(tv_session_id, NULL);
    if(ret != MSP_SUCCESS){
        fprintf(stderr, "QTTSSessionEnd failed. Error code %d.\n", ret);
    }

	MSPLogout();
	printf("Press Enter to exit...\n");
	getchar();
	return 0;
}

