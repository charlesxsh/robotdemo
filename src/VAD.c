#include <math.h>
#include <stdio.h>
#include "params.h"
#include "VAD.h"
#include "stdlib.h"
#include <memory.h> 
//////////////////////////////////////////////////////////

#define ENERGY_LIST_SIZE	300
#define KIBASE				0.48

char	m_buffAgg[1000];  ///加重后的信号
int     m_buffLen = 1000;

int     m_last;         ///前一帧最后一个采样信号        
float m_crossZeroRate;  ///过零率
long  m_littleEnergy;   ///短时平均能量
long  m_energyList[ENERGY_LIST_SIZE] = {80};  ///N个平均能量
int   m_energyIdx = 0;
long  m_energySum;              ///N个平均能量和
float m_thresholdCross;     ///过零率门限
long  m_thresholdUpEnergy;  ///短时平均能量开门限
long  m_thresholdDownEnergy;///短时平均能量关门限
long  m_upCount;        ///连续处于打开状态次数
long  m_downCount;      ///连续处于关闭状态次数
volatile bool  m_hasVoice;       ///是否有语音的开关状态
volatile bool  m_hasVoiceRT;
//        time_t m_lastUpdateTime;
volatile float m_sensitive;      ///检测的灵敏度，值越小越灵敏(0.02~0.1)
volatile bool m_enable;

/////////////////////////////////////////////////////////


void updateCrossZeroRate(const char* buff, int len)
{
    int crossZeroCount = 0;
    char state = 'a';
    short lv=10;
    short v=10;
    short v1;
    m_crossZeroRate = 1.0;
    float minThreshold = 3500*m_sensitive-50;//0.1~300//0.02~20;

    short* sbuff = (short*)buff;
    int i;
    for (i=0; i<len/2; ++i)
    {
        v1 = sbuff[i];
        if (abs(v1)<minThreshold) v1=0;
        switch (state)
        {
        case 'a':
            if (v1<0)
            {
                state = 'b';
                ++crossZeroCount;
            }
            break;
        case 'b':
            if (v1>0)
            {
                state = 'd';
                ++crossZeroCount;
            }
            else
            {
                state = 'c';
            }
            break;
        case 'c':
            if (v1>0)
            {
                state = 'd';
                ++crossZeroCount;
            }
            break;
        case 'd':
            if (v1<0)
            {
                state = 'b';
                ++crossZeroCount;
            }
            else
            {
                state = 'a';
            }
            break;
        }
    }
    m_crossZeroRate = crossZeroCount*2.0F/len;
}
void updateLittleEnergy(const char* buff, int len)
{
    const float k = 0.95F;
    const int upper = 32768;
    double energy;
    //语音信号预加重  x(n) = X(n) - kX(n-1)
    short* X = (short*)buff;
    short* x = (short*)m_buffAgg;
    int size = len/2;
    x[0] = (short)(X[0]-k*m_last);
    energy = abs(X[0]);
    int i;
    for (i=1; i<size; ++i)
    {
        x[i] = (short)(X[i]-k*X[i-1]);
        energy += (abs(X[i]));
    }
    energy/=size;//平均能量

    m_littleEnergy = energy;
    m_last = X[size-1];
        
        
    //动态调整门限
    float KI = (KIBASE+m_sensitive*2)/200;
    if (m_hasVoice && m_crossZeroRate > m_thresholdCross)
    {
        m_energySum += energy;
		m_energySum -= m_energyList[m_energyIdx];
		m_energyList[m_energyIdx] = energy;
		m_energyIdx++;
		if(m_energyIdx == ENERGY_LIST_SIZE)
		{
			m_energyIdx = 0;
			
		}
		

        m_thresholdUpEnergy = m_energySum*KI;
        m_thresholdDownEnergy = m_thresholdUpEnergy*0.8;
    }
}

void detectVoice()
{
    if (m_crossZeroRate > m_thresholdCross && m_upCount>1)
    {
        if (m_downCount>0) m_downCount -= 1;
    }
        
    if (m_littleEnergy < m_thresholdDownEnergy)
    {
        ++m_downCount;
    }
    else
    {
        m_downCount = 0;
    }
        
    if (m_downCount > 400)
    {
        m_hasVoice = 0;
        m_upCount = 0;
    }

    if (m_downCount > 30)
    {
        m_hasVoiceRT = 0;
    }
        
        
    if (m_littleEnergy > m_thresholdUpEnergy)
    {
        m_upCount++;
        m_hasVoiceRT = 1;
    }
    else
    {
        m_upCount = 0;
    }
    if (m_upCount>0 && m_crossZeroRate > m_thresholdCross)
    {
        m_hasVoice = 1;
        m_downCount = 0;
    }
        
    //LOGI("VoiceDetectionOutput m_hasVoice[%s]m_hasVoiceRT[%s] m_upCount[%d] m_downCount[%d]", m_hasVoice ? "1" : "0", m_hasVoiceRT ? "1" : "0", m_upCount, m_downCount);
//         if (m_upCount>0 && m_crossZeroRate > m_thresholdCross/3)
//         {
//             m_hasVoiceRT = 1;
//         }
}

bool hasVoiceRT()
{
    return m_enable?(m_hasVoiceRT&&m_hasVoice):1;
}

void _vad_write(const char* buff, int len)
{
    /*
    bool flag = 0;
    for(int j = 0; j < len; j++)
    {
        if (buff[j] != 0)
        {
            flag = 1;
            break;
        }
    }
    LOGI("VoiceDetectionOutput %s zero data", flag ? "has not" : "all");
    */
    if(len <= 0)
	{
		return ;
	}

    int l = len/4;
    int i;
    for (i=0;i<4;++i)
    {
        updateCrossZeroRate(buff+i*l, l);
        updateLittleEnergy(buff+i*l, l);
        detectVoice();
    }
}


void _vad_reset(void)
{
	memset(m_buffAgg, 0x00, sizeof(m_buffAgg));	
	m_last = 0;
	m_crossZeroRate = 1.0;
	m_littleEnergy = 0;
	m_thresholdCross  = 0.05;
	m_enable = 1;
	m_sensitive = 0.16; //0.04

    m_thresholdCross  = 0.05;
    m_upCount = 0;
    m_downCount = 0;
    m_hasVoice = 0;
    m_hasVoiceRT = 0;
    m_energySum = 80*ENERGY_LIST_SIZE;
    float KI = (KIBASE+m_sensitive*2)/200;
    m_thresholdUpEnergy = m_energySum*KI;
    m_thresholdDownEnergy = m_thresholdUpEnergy*0.8;
}

bool vad_detect(char* data, int len)
{
	_vad_write(data, len);
	return hasVoiceRT();
}

/////////////////////////////////////////////////////////
/*
typedef enum
{
	Nothing,
	IsEnd,
	TimeOut
}vad_state_t;
*/

bool mLastVad = 0;
long mStartTime = 0;
int TIMEOUT = 1000;
bool mRunning = 1;
unsigned int win_cnt = 0;

vad_state_t detect(char *data, int len)
{
	win_cnt++;
	if(!mRunning) 
		return Nothing;
	vad_state_t bRt = Nothing;
		
	bool bVad = vad_detect(data, len);
	//刚起动时会一直返回1
		
	if(mLastVad == 1 && bVad == 0)
	{
		bRt =  IsEnd;
		//get current tick
		//mStartTime = System.currentTimeMillis();
	}
	else
	{
		if(win_cnt > TIMEOUT)
		{
			bRt =  TimeOut;
			win_cnt = 0;
		}
	}
		
	if(mLastVad != bVad)
	{
		win_cnt = 0;
	}
		
	mLastVad = bVad;
	return bRt;
}
