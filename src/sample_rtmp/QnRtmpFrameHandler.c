#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include "rtmp_publish.h"
#include "QnCommon.h"
#include "QnMqttConn.h"
#include "QnRtmpFrameHandler.h"
#include "OnMsg.h"
#include "rtmp_publish.h"

#define RTMPURL_LEN (256)
#define MAX_BUF_SIZE (1024*1024*10)

static int NaluAlloc(OUT Nalu *pNalu, IN const char *pData, IN int nlen);
static void NaluFree(IN Nalu *pNalu);
static int NaluCopy(IN Nalu *pNalu, const char *pData, int nLen);
	
static Nalu* ParseNalu(IN const char *pStart, IN int nlen);
static void DestroyNalu(IN Nalu **nalu);

static int AdtsAlloc(OUT Adts *pAdts, IN const char *pData, IN int nLen);  
static void AdtsFree(IN Adts *pAdts);
static Adts* ParseAdts(IN char *pStart, int nLen);
static void DestroyAdts(IN Adts ** _adts);
static void MqttSendRtmpLog( char *_pFuncName, int val, int _nIsError,
                             struct timeval *_pStartTime, struct timeval *_pEndTime );

unsigned int gTotalSendBytes = 0;
pthread_mutex_t gMqttMutex;

static struct RtmpFrameHandler *rtmphandler = NULL;

int GetTimeDiff( struct timeval *_pStartTime, struct timeval *_pEndTime )
{
    int time = 0;

    if ( _pEndTime->tv_sec < _pStartTime->tv_sec ) {
        return -1;
    }

    if ( _pEndTime->tv_usec < _pStartTime->tv_usec ) {
        time = (_pEndTime->tv_sec - 1 - _pStartTime->tv_sec) +  
            ((1000000-_pStartTime->tv_usec) + _pEndTime->tv_usec)/1000000;
    } else {
        time = (_pEndTime->tv_sec - _pStartTime->tv_sec) + 
            (_pEndTime->tv_usec - _pStartTime->tv_usec)/1000000;
    }

    return ( time );

}

static void SigPipeHandler(int nSigno)
{
    QnDemoPrint(DEMO_INFO, "%s:%d>> Signal number:%d\n", __func__, __LINE__, nSigno);
    // RtmpStatus(RTMP_STOP);
    // RtmpStatus(RTMP_START);
}

#define NUM_LINE 8
static void PrintPck(IN unsigned char *_pAddr, IN int _nLen)
{
	int i = 0;
	
	printf("========== memory <%p+%lu> ==========", _pAddr, _nLen);
 	for (; i < _nLen; ++i) {
		   if (i % NUM_LINE == 0) {
				   printf("\n");
				   printf("<%p>:", _pAddr + i - NUM_LINE);
		   }

		   printf("0x%02x\t", _pAddr[i]);
   }
	printf("\n======== memory <%p+%lu> end ========\n", _pAddr, _nLen);
}

static int NaluAlloc(IN Nalu *_pNalu, IN const char *_pData, IN int _nLen)
{
	if (_pNalu == NULL || _pData == NULL)
	{
		return QN_FAIL;
	}
	_pNalu->data = (char *)malloc(_nLen);
	if (_pNalu->data == NULL)
	{
		return QN_FAIL;
	}
	memcpy(_pNalu->data, _pData, _nLen);
	_pNalu->size = _nLen;
	
	return QN_SUCCESS;
}

static void NaluFree(IN Nalu *_pNalu)
{
	if (_pNalu)
	{
		if (_pNalu->data)
		{
			free(_pNalu->data);
			_pNalu->data = NULL;
			_pNalu->size = -1;
		}
		
	}
}

static int NaluCopy(IN Nalu *_pNalu, IN const char *_pData, IN int _nlen)
{
	if (_pNalu == NULL || _pData == NULL)
	{
		return QN_FAIL;
	}
	if (_pNalu->size < _nlen)
	{
		return QN_FAIL;
	}
	memcpy(_pNalu->data, _pData, _nlen);

	return QN_SUCCESS;
}

static Nalu* ParseNalu(IN const    char *_pStart, IN int _nLen)
{
	if (_nLen <= 0) {
		return NULL;
	}
	if (!_pStart)
	{	
		return NULL;
	}
	const char* pStart = _pStart;
	const char *pEnd = NULL;
	Nalu *nalu = NULL;

	while (pStart < _pStart + _nLen)
	{
		if (pStart >= _pStart + _nLen - 4) {
			pStart = _pStart + _nLen;
			break;
		}

		if (pStart[0] == 0x00 
			&& pStart[1] == 0x00
			&& pStart[2] == 0x00
			&& pStart[3] == 0x01) {

			pStart = pStart + 4;
			pEnd = pStart;
			while (pEnd < _pStart + _nLen )
			{
				if (pEnd[0] == 0x00
					&& pEnd[1]== 0x00
					&& pEnd[2] == 0x00
					&& pEnd[3] == 0x01)
				{
					goto NEXT;
				}
				else 
				{
					pEnd = pEnd + 1;
				}
			}
			if (pEnd >= _pStart + _nLen)
			{
				break;
			}
		} else {
				pStart = pStart + 1;
		}
	}
	

NEXT:
	if (pStart == _pStart + _nLen) {
		QnDemoPrint(DEMO_WARNING, "%s[%d] pStart:%p start+len:%p\n", __func__, __LINE__, pStart, _pStart + _nLen);
		return NULL;
	}
	
	nalu = malloc(sizeof(Nalu));
	if (!nalu) {
		return NULL;
	}
	NaluAlloc(nalu, pStart, (int)(pEnd - pStart));

	return nalu;
}

static void DestroyNalu(IN Nalu** nalu)
{
	NaluFree(*nalu);
	free(*nalu);
	*nalu = NULL;
}

static int AdtsAlloc(IN Adts *_pAdts, IN const char *_pData, IN int _nLen)
{
	if (_pAdts == NULL || _pData == NULL) {
		QnDemoPrint(DEMO_WARNING, "%s[%d] null parameters.\n", __func__, __LINE__);
		return QN_FAIL;
	}

	_pAdts->data = (char *)malloc(_nLen);
	if (!_pAdts->data) {
		QnDemoPrint(DEMO_WARNING, "%s[%d] malloc failed.\n", __func__, __LINE__);
		return QN_FAIL;
	}
	memcpy(_pAdts->data, _pData, _nLen);
	_pAdts->size = _nLen;

	return QN_SUCCESS;
}

static void AdtsFree(OUT Adts *_pAdts)
{
	if (_pAdts) {
		if (_pAdts->data != NULL) {
			free(_pAdts->data);
			_pAdts->data = NULL;
			_pAdts->size = -1;
		}
	}
}

static Adts* ParseAdts(IN char *_pStart, IN int _nLen)
{
	if (_pStart == NULL) {
		QnDemoPrint(DEMO_WARNING, "%s[%d] parameter null.\n", __func__, __LINE__);
		return NULL;
	}

	int headLen = 0; 
	const char *pStart = _pStart; 
	const char *pEnd = NULL;  
	
	while (pStart < _pStart + _nLen) {

		if (pStart + 2 >= _pStart + _nLen) {
			break;
		}

		if (pStart[0] == 0xff
			&& (pStart[1] & 0xf0) == 0xf0) {
			headLen = (pStart[1] & 0x1) == 1 ? 7 : 9; 
			break;
		} else {
			pStart++;
		}
	}

ADTS_NEXT:
	if (pStart >= _pStart + _nLen -2) {
		return NULL;
	}
	
	int frameLen = 0;         
	int aacFrameLen = 0; 

	aacFrameLen |= (pStart[3] & 0x3) << 11;
	aacFrameLen |= pStart[4] << 3;
	aacFrameLen |= (pStart[5] & 0xe0) >> 5;
	frameLen = (int)(pEnd - pStart);
	Adts *adts = (Adts *)malloc(sizeof(Adts));
	if (adts == NULL) {
		QnDemoPrint(DEMO_WARNING, "%s[%d] malloc failed.\n", __func__, __LINE__);
		return NULL;
	}
	if (AdtsAlloc(adts, _pStart, aacFrameLen) == QN_FAIL) {
		free(adts);
		adts = NULL;
		return NULL;
	}
	return adts;
}

static void DestroyAdts(IN Adts ** _adts)
{
	AdtsFree(*_adts);
	free(*_adts);
	*_adts = NULL;
}

int GetAdtsFHL(IN const Adts *_pAdts, OUT int *_nHeadLen)
{
	if (_pAdts == NULL || _nHeadLen == NULL) {
		return QN_FAIL;
	}

	if ((_pAdts->data[1] & 0x1) == 1) {
		*_nHeadLen = 7;
	} else {
		*_nHeadLen = 9;
	}
	
	return QN_SUCCESS;
}

/*
 * function: rtmp_init
 * description: init rtmp context
 * */
static int rtmp_init(struct FrameHandler *handler)
{
	
	struct RtmpFrameHandler *rtmpHandler = container_of(handler, struct RtmpFrameHandler, base);
	if (rtmpHandler->pRtmpCtx) {
		QnDemoPrint(DEMO_ERR, "%s:%d>> release rtmp context.\n", __func__, __LINE__);
		RtmpPubDel(rtmpHandler->pRtmpCtx);
	}

	char *url = rtmpHandler->url;
	do {
		usleep(200);
		rtmpHandler->pRtmpCtx = RtmpPubNew(url, 10, RTMP_PUB_AUDIO_AAC, RTMP_PUB_AUDIO_AAC,\
											RTMP_PUB_TIMESTAMP_ABSOLUTE);
		if (!rtmpHandler->pRtmpCtx) {
			QnDemoPrint(DEMO_ERR, "%s:%d>> Get rtmp context fail.\n", __func__, __LINE__);
			return QN_FAIL;
		}
		if (RtmpPubInit(rtmpHandler->pRtmpCtx) != QN_SUCCESS) {
			goto RELEASE_RTMPCTX;
		}
		if (RtmpPubConnect(rtmpHandler->pRtmpCtx) != QN_SUCCESS) {
			goto RELEASE_RTMPCTX;
		} else {
			QnDemoPrint(DEMO_ERR, "%s:%d>> Get Rtmp context succeed.\n", __func__, __LINE__);
			break;
		}

RELEASE_RTMPCTX:
		RtmpPubDel(rtmpHandler->pRtmpCtx);
		rtmpHandler->pRtmpCtx = NULL;
		continue;

	} while (1);
	rtmpHandler->videoState = QN_FALSE;
	rtmpHandler->audioState = QN_FALSE;
	rtmpHandler->isOk = QN_FALSE;
	rtmpHandler->state = RTMP_START;

	return QN_SUCCESS;
}

/*
 * function: rtmp_release
 * dexcription: release rtmp context
 * */
static void rtmp_release(struct FrameHandler *handler)
{
	QnDemoPrint(DEMO_ERR, "%s:%d>> ====\n", __func__, __LINE__);

	struct RtmpFrameHandler *rtmpHandler = container_of(handler, struct RtmpFrameHandler, base);

	RtmpPubDel(rtmpHandler->pRtmpCtx);
	rtmpHandler->videoState = QN_FALSE;
	rtmpHandler->audioState = QN_FALSE;
	rtmpHandler->state = RTMP_STOP;
	rtmpHandler->isOk = QN_FALSE; //rtmp meta data, send status
	rtmpHandler->pRtmpCtx = NULL;
}

struct RtmpFrameHandler *rtmp_gethandler()
{
	return rtmphandler;
}

/*
 * function: rtmp_reinit
 * description: reinit rtmp context
 * */
static void rtmp_reinit(struct FrameHandler *handler)
{
	rtmp_release(handler);
	rtmp_init(handler);
	QnDemoPrint(DEMO_ERR, "%s:%d>> reinit success.\n", __func__, __LINE__);
}

static void MqttSendRtmpLog( char *_pFuncName, int val, int _nIsError,\
                             struct timeval *_pStartTime, struct timeval *_pEndTime )
{
    // ErrorID ret = 0;
    char reportStr[128] = { 0 };
    char *errorStr = NULL;
    char log[64] = {0};
    int duration = 0, timeArrive = 0;

    if ( !_pFuncName ) {
        DBG_LOG("_pFuncName is null\n");
        return;
    }

    gettimeofday( _pEndTime, NULL );
    duration = GetTimeDiff( _pStartTime, _pEndTime  );
	 // DBG_LOG("duration = %d\n", duration );
    if ( duration >= 5) {
        if ( _nIsError ) {
            errorStr = strerror(errno);
            strcat( reportStr, "[ RTMPERR ][ ");
            strcat( reportStr, _pFuncName );
            strcat( reportStr, " ][ " );
            strcat( reportStr, errorStr );
            strcat( reportStr, " ]");
        } else {
            if ( val != 0) {
				struct OnMsg *msg = NewOnMsg("001", "RTMPV", val);
				struct RtmpFrameHandler *handler = rtmp_gethandler();
				struct MqttConn *conn = container_of(handler->conn, struct MqttConn, base);
				conn->base.ops->ConnWrite(&conn->base, msg->base.elt.data, sizeof(msg->base.elt.data));
				DeleteOnMsg(msg);
            }
        }
        // ret = Report( gMqttCtx.accountId, gTopic, reportStr, strlen(reportStr) );
        // if ( ret != RET_OK ) {
        //     DBG_LOG("Report error\n");
        // }
        *_pStartTime = *_pEndTime;
    } 

}

/*
 * function: rtmp_video_send
 * description: rtmp channel to send video&H264
 * */
static int rtmp_video_send(char *data, int len, double timestamp, int iskey)
{
	if (data == NULL) {
		QnDemoPrint(DEMO_ERR, "%s:%d>> data is null.\n", __func__, __LINE__);
		return QN_FAIL;
	}
	
	// char log[64] = {0};
	static struct timeval start = {0, 0},end;
	// itoa(len, log);
	MqttSendRtmpLog("RtmpH264Send", len, 0, &start, &end);

	QnDemoPrint(DEMO_INFO, "%s:%d>> =====\n", __func__, __LINE__);

	struct RtmpFrameHandler *rtmpHandler = rtmp_gethandler();
	long long int presentationTime = (long long int)timestamp;
	Nalu *pNalu = NULL;
	int s32Ret = QN_FAIL;

	if (rtmpHandler->state == RTMP_START) {

		char *buf = (char *)malloc(MAX_BUF_SIZE);
		if (!buf) {
			QnDemoPrint(DEMO_WARNING, "%s[%d] malloc failed for buf.\n", __func__, __LINE__);
			return QN_FAIL;
		}

		int nIdx = 0;
		//第一次时，设置sps、pps
		if (rtmpHandler->videoState == QN_FALSE && iskey) {
			RtmpPubSetVideoTimebase(rtmpHandler->pRtmpCtx, presentationTime);
			pNalu = ParseNalu(data, len);
			if (!pNalu) {
				goto RTMP_VIDEO_SEND_FREE_EXIT;
			}
			RtmpPubSetSps(rtmpHandler->pRtmpCtx, pNalu->data, pNalu->size);
			nIdx += pNalu->size + 4;
			DestroyNalu(&pNalu);

			pNalu = ParseNalu(data + nIdx, len - nIdx );
			if (!pNalu) {
				goto RTMP_VIDEO_SEND_FREE_EXIT;
			}
			RtmpPubSetPps(rtmpHandler->pRtmpCtx, pNalu->data, pNalu->size);
			nIdx += pNalu->size + 4;
			DestroyNalu(&pNalu);

			rtmpHandler->videoState = QN_TRUE;
			rtmpHandler->isOk = QN_TRUE;
			QnDemoPrint(DEMO_INFO, "%s:%d>> set sps, pps.\n", __func__, __LINE__);
		}

		//video i,p帧数据
		#if 1
		int bufSize = 0;
		while (nIdx < len) {
			pNalu = ParseNalu(data + nIdx, len - nIdx);
			if (!pNalu) {
				QnDemoPrint(DEMO_WARNING, "%s:%d>> ParseNalu failed.\n", __func__, __LINE__);
				break;
			}
				
			if (bufSize + pNalu->size > MAX_BUF_SIZE) {
				QnDemoPrint(DEMO_WARNING, "%s:%d>> malloc failed for buf\n", __func__, __LINE__);
				DestroyNalu(&pNalu);
				goto RTMP_VIDEO_SEND_FREE_EXIT;
			}
				
			if (((pNalu->data[0] & 0x0f) == 0x01) || ((pNalu->data[0] & 0x0f) == 0x05)) {
				
				if (pNalu->size < 0) {
					QnDemoPrint(DEMO_WARNING, "%s:%d>>  nalU->size < 0\n", __func__, __LINE__);
					goto RTMP_VIDEO_SEND_FREE_EXIT;
				}
				memcpy(&buf[bufSize], pNalu->data, pNalu->size);
				bufSize += pNalu->size;
				if (bufSize >= len) {
					QnDemoPrint(DEMO_WARNING, "%s:%d>> bufSize >= textLen\n", __func__, __LINE__);
					goto RTMP_VIDEO_SEND_FREE_EXIT;
				}
			}
			nIdx += pNalu->size + 4;
			DestroyNalu(&pNalu);
		}
			
		if (bufSize == 0) {
			QnDemoPrint(DEMO_NOTICE, "%s:%d>> bufSize == 0.\n", __func__, __LINE__);
			goto RTMP_VIDEO_SEND_FREE_EXIT;
		}

		//发送video数据
		if (iskey) {
			if (RtmpPubSendVideoKeyframe(rtmpHandler->pRtmpCtx, buf, bufSize, presentationTime) != QN_SUCCESS) {
				QnDemoPrint(DEMO_ERR, "%s:%d>> Send video key frame fail, reInit Rtmp.\n", __func__, __LINE__);
				goto RTMP_VIDEO_SEND_REINIT_EXIT;
			}
		}
		else {
			if (RtmpPubSendVideoInterframe(rtmpHandler->pRtmpCtx, buf, bufSize, presentationTime) != QN_SUCCESS) {
				QnDemoPrint(DEMO_ERR, "%s:%d>> Send video inter frame fail, reInit Rtmp.\n", __func__, __LINE__);
				goto RTMP_VIDEO_SEND_REINIT_EXIT;
			}
		}
	#endif
	
	s32Ret = QN_SUCCESS;
	goto RTMP_VIDEO_SEND_FREE_EXIT;
		
RTMP_VIDEO_SEND_REINIT_EXIT:
	rtmp_reinit(&rtmpHandler->base);
	// exit(1);

RTMP_VIDEO_SEND_FREE_EXIT:
			if (buf) {
				free(buf);
			}
	}
RTMP_VIDEO_SEND_EXIT:
	return s32Ret;

}

/*
 * function: rtmp_audio_send
 * description: rtmp channel to send audio&aac
 * */
static int rtmp_audio_send(char *data, int len, double timestamp, unsigned int type)
{
	static unsigned char audioSpecCfg[] = {0x14, 0x10}; 
	long long int presentationTime = (long long int)timestamp;

	if (!data) {
		return QN_FAIL;
	}

	QnDemoPrint(DEMO_ERR, "%s:%d>> ====in ====\n", __func__, __LINE__);
	struct RtmpFrameHandler *rtmpHandler = rtmp_gethandler();
	RtmpPubContext *pRtmpc = rtmpHandler->pRtmpCtx;
	int nRet = QN_FAIL;


	if (rtmpHandler->state == RTMP_START) {

		//等待video发送meta data
		if (rtmpHandler->isOk == QN_FALSE) {
			//QnDemoPrint(DEMO_INFO, "%s[%d] wait for video metadata send.\n", __func__, __LINE__);
			usleep(500);
			goto RTMP_AUDIO_SEND_EXIT;
		}

		//aac 设置解码参数
		if (rtmpHandler->audioState == QN_FALSE) {
			RtmpPubSetAudioTimebase(pRtmpc, presentationTime);
			if (type == AUDIO_TYPE_AAC) {
				RtmpPubSetAac(pRtmpc, audioSpecCfg, sizeof(audioSpecCfg));
			}
			rtmpHandler->audioState = QN_TRUE;
		}
		
		int uIdx = 0;
		int bufSize = 0;
		int headLen = 0;
		Adts *adts = NULL;
		char *buf = NULL;

		buf = (char *)malloc(len);
		if (buf == NULL) {
			QnDemoPrint(DEMO_WARNING, "%s:%d>> malloc buffer failed.\n", __func__, __LINE__);
			goto RTMP_AUDIO_SEND_EXIT;
		}

		while (uIdx < len) {
			
			adts = ParseAdts(data + uIdx, len - uIdx);
			if (!adts) {
				goto RTMP_AUDIO_SEND_FREE_EXIT;
				// goto RTMPAUDIO_FREE_UNLOCK;
			}
			//adts header Ϊ7bytes
			if (GetAdtsFHL(adts, &headLen) == QN_FAIL) {
				QnDemoPrint(DEMO_INFO, "%s:%d>>  getAdtsFHL failed.\n", __func__, __LINE__);
				DestroyAdts(&adts);
				goto RTMP_AUDIO_SEND_FREE_EXIT;
			}
			if ((adts->size - headLen) < 0) {
				DestroyAdts(&adts);
				QnDemoPrint(DEMO_INFO, "%s:%d>> adts->size - headLen < 0\n", __func__, __LINE__);
				goto RTMP_AUDIO_SEND_FREE_EXIT;
			}
			memcpy(&buf[bufSize], &adts->data[headLen], adts->size - headLen);
			bufSize += adts->size - headLen;
			if (bufSize >= len) {
				QnDemoPrint(DEMO_WARNING, "%s:%d>> bufSize >= textLen\n", __func__, __LINE__);
				DestroyAdts(&adts);
			    goto RTMP_AUDIO_SEND_FREE_EXIT;
			}
			uIdx = uIdx + adts->size;
			DestroyAdts(&adts);
		}
		nRet = RtmpPubSendAudioFrame(pRtmpc, buf, bufSize, presentationTime);
		if (nRet != QN_SUCCESS) {
			QnDemoPrint(DEMO_ERR, "%s:%d>> RtmpPubSendAudioFrame fail.\n", __func__, __LINE__);
			goto RTMP_AUDIO_SEND_REINIT;
		}
        // if ( nRet != 0 ) {
        //     static struct timeval start, end; 

        //     MqttSendRtmpLog( "RtmpPubSendAudioFrame", NULL, 1, &start, &end );
        // }

		nRet = QN_SUCCESS;
RTMP_AUDIO_SEND_REINIT:
		QnDemoPrint(DEMO_ERR, "%s:%d>>!!!!!!!\n", __func__, __LINE__);
RTMP_AUDIO_SEND_FREE_EXIT:
		if (buf) {
			free(buf);
			buf = NULL;
		}
	}
	
RTMP_AUDIO_SEND_EXIT:
	// pthread_mutex_unlock(&gAppContext.pushLock);
	
	return nRet;
}

static const struct FrameHandlerOperations gRtmpFrameHandlerOps = {
	// .Open = rtmp_open,
	// .Close = rtmp_close,
	.SendVideo = rtmp_video_send,
	.SendAudio = rtmp_audio_send,
};

struct RtmpFrameHandler *NewRtmpFrameHandler(struct RtmpConf *cfg)
{
	if (!rtmphandler) {
		struct MqttConn *conn = NewMqttConn();
		if (!conn) {
			return NULL;
		}
		rtmphandler = (struct RtmpFrameHandler *)malloc(sizeof(struct RtmpFrameHandler));
		if (rtmphandler) {
			memset(rtmphandler, 0, sizeof(struct RtmpFrameHandler));
			rtmphandler->pRtmpCtx = NULL;
			rtmphandler->url = cfg->url;
			rtmphandler->conn = &conn->base;
			rtmphandler->base.ops = &gRtmpFrameHandlerOps;
			rtmp_init(&rtmphandler->base);
		}
	}
	if (rtmphandler) {
		rtmphandler->counter++;
	}
	QnDemoPrint(DEMO_INFO, "%s:%d>> rtmphandler->counter :%d\n", __func__, __LINE__, rtmphandler->counter);
	return rtmphandler;
}

void DeleteRtmpFrameHandler(struct RtmpFrameHandler *rtmphandler)
{
	QnDemoPrint(DEMO_ERR, "%s:%d>> rtmphandler->counter :%d\n", __func__, __LINE__, rtmphandler->counter -1);
	
	rtmphandler->counter--;
	if (rtmphandler->counter) {
		return;
	}
	struct MqttConn *mqttconn = container_of(rtmphandler->conn, struct MqttConn, base); 
	rtmp_release(&rtmphandler->base);
	DeleteMqttConn(mqttconn);
	free(rtmphandler);
	rtmphandler = NULL;
}
