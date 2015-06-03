#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "msp_cmn.h"
#include "qivw.h"
#include "msp_errors.h"

#define USR "charlesxsh@163.com"
#define PWD "Xsh952644081"


int main(int argc, char *argv[]){
	int ret;
	const char *usr = USR;
	const char *pwd = PWD;
	const char *lgi_param = "appid = 55657997";
	ret = MSPLogin(usr, pwd, lgi_param);
	if(ret != MSP_SUCCESS){
		printf("MSPLogin failed, error code is: %d\n", ret);
		exit(1);
	}
    //voice to text
	const char *vt_params = "vcn=xiaoyan,spd=50,text_encoding=UTF8,ttp=text,vol=50,background_sound=1";
    //text to voice
    const char *tv_params = "engine_type=cloud,domain=iat,aue=raw;-1,auf=audio/L16;rate=8000,vad_timeout=1000,vad_speech_tail=1000,asr_ptt=false,result_type=plain,result_encoding=utf-8";
	//text_to_voice("说英语可以吗", vt_params, "test.raw");
    const char *text = voice_to_text(tv_params, "test.raw");
    printf("%s\n", text);
	MSPLogout();
	printf("\n按任意键退出...\n");
	getchar();
	return 0;
}

