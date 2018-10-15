#ifndef __QNRTMP_H__
#define __QNRTMP_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>

#include "rtmp_publish.h"
#include "QnCommon.h"


#define AUDIO_TYPE_G711 (0)
#define AUDIO_TYPE_AAC (1)


#define NAL_SLICE 1          
#define NAL_SLICE_IDR  5        
#define NAL_SPS  7              //SPS֡
#define NAL_PPS  8              //PPS֡

#define QU_LEN 128  

#define RTMP_START  (1)
#define RTMP_STOP (0)
                                    

typedef struct
{
	int size;
	char *data;
} Nalu;

typedef struct _AdtsUint
{
	unsigned int size;
	unsigned char *data;
}Adts;

typedef struct _pagAppContext
{
	RtmpPubContext *pRtmpc;

	int status;
	int isOk;
	int videoState;
	int audioState;
	pthread_mutex_t pushLock;
	int (*RtmpH264Send)(char *pData, int nLen, double dTimeStamp, int _nIskey);
	int (*RtmpAudioSend)(char *pData, int _nLen, double nTimeStamp, unsigned int uAudioType);
}AppContext;

// add by liyq
extern unsigned int gTotalSendBytes;
extern pthread_mutex_t gMqttMutex;

void InitSigHandler(void);
int RtmpInit();
void RtmpRelease();
void RtmpStatus(IN int _nStatus);
int VideoCallBack(IN int nStreamNO, IN char *pFrame, IN int nLen, 
	IN int nIskey, IN double dTimestamp, IN unsigned long ulFrameIndex,	
	IN unsigned long ulKeyFrameIndex, void *pContext);
int AudioCallBack(IN char *pFrame, IN int nLen, IN double dTimestamp, IN unsigned long ulFrameIndex, void *pContext);
void InitMqttMutex();
void DestroyMqttMutex();
extern pthread_mutex_t gMqttMutex;



#ifdef  __cplusplus
}
#endif
#endif

