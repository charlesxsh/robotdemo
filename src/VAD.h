
#ifndef _VAD_H_
#define _VAD_H_
typedef enum
{
	Nothing,
	IsEnd,
	TimeOut
}vad_state_t;


#define bool int

void updateCrossZeroRate(const char* buff, int len);
void updateLittleEnergy(const char* buff, int len);
void detectVoice();
bool hasVoiceRT();
void _vad_write(const char* buff, int len);
void _vad_reset(void);
bool vad_detect(char* data, int len);
vad_state_t detect(char *data, int len);

#endif 
