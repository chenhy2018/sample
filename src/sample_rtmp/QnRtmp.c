#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include "sdk_interface.h"
#include "QnMqtt.h"
#include "QnRtmp.h"


#define RTMPURL_LEN (256)

static int NaluAlloc(OUT Nalu *pNalu, IN const char *pData, IN int nlen);
static void NaluFree(IN Nalu *pNalu);
static int NaluCopy(IN Nalu *pNalu, const char *pData, int nLen);
	
static Nalu* ParseNalu(IN const char *pStart, IN int nlen);
static void DestroyNalu(IN Nalu **nalu);

static int AdtsAlloc(OUT Adts *pAdts, IN const char *pData, IN int nLen);  
static void AdtsFree(IN Adts *pAdts);
static Adts* ParseAdts(IN char *pStart, int nLen);
static void DestroyAdts(IN Adts ** _adts);
static void MqttSendRtmpLog( char *_pFuncName, char *_pLog, int _nIsError,
                             struct timeval *_pStartTime, struct timeval *_pEndTime );

AppContext gAppContext;
unsigned int gTotalSendBytes = 0;
pthread_mutex_t gMqttMutex;

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

int VideoCallBack(IN int _nStreamNO, IN char *_pFrame, IN int _nLen, 
	IN int _nIskey, IN double _dTimestamp, IN unsigned long _ulFrameIndex,	
	IN unsigned long _ulKeyFrameIndex, void *_pContext)
{
	static unsigned int nLastTimeStamp = 0;
	int *pStreamNo = (int*)_pContext;
	AppContext *appCtx = (AppContext *)_pContext;
	int streamno = *pStreamNo;
	int nDiff = (unsigned int)_dTimestamp - nLastTimeStamp;

    /*DBG_LOG("VideoCallBack()\n");*/

	nLastTimeStamp = (unsigned int)_dTimestamp;
	if (gAppContext.RtmpH264Send == NULL) {
		if (_nIskey)
		    QnDemoPrint(DEMO_WARNING, "%s[%d] g_AContext.RtmpH264Send is NULL.\n", __func__, __LINE__);
		return QN_FAIL;
	}
	gAppContext.RtmpH264Send(_pFrame, _nLen, _dTimestamp, _nIskey);
	return QN_SUCCESS;
}

int AudioCallBack(IN char *_pFrame, IN int _nLen, IN double _dTimestamp, IN unsigned long _ulFrameIndex, void *_pContext)
{
	if (!gAppContext.RtmpAudioSend) {
		return QN_SUCCESS;
	}
	
	gAppContext.RtmpAudioSend(_pFrame, _nLen, _dTimestamp, RTMP_PUB_AUDIO_AAC);

	return 0;
}

static void SigPipeHandler(int nSigno)
{
    QnDemoPrint(DEMO_INFO, "Signal number:%d\n", nSigno);
    RtmpStatus(RTMP_STOP);
    RtmpStatus(RTMP_START);
}

static void MqttSendRtmpLog( char *_pFuncName, char *_pLog, int _nIsError,
                             struct timeval *_pStartTime, struct timeval *_pEndTime )
{
    ErrorID ret = 0;
    char reportStr[128] = { 0 };
    char *errorStr = NULL;
    char log[64] = {0};
    int duration = 0, timeArrive = 0;

    if ( !_pFuncName ) {
        DBG_LOG("_pFuncName is null\n");
        return;
    }

    gettimeofday( _pEndTime, NULL );
    /*DBG_LOG("_pStartTime->tv_sec = %d\n", _pStartTime->tv_sec );*/
    /*DBG_LOG("_pStartTime->tv_usec = %d\n", _pStartTime->tv_usec );*/
    /*DBG_LOG("_pEndTime->tv_usec = %d\n", _pEndTime->tv_usec );*/
    /*DBG_LOG("_pEndTime->tv_sec = %d\n", _pEndTime->tv_sec );*/
    duration = GetTimeDiff( _pStartTime, _pEndTime  );
    /*DBG_LOG("duration = %d\n", duration );*/
    if ( duration >= 5 && gMqttAlreadyConnecgted) {
        if ( _nIsError ) {
            errorStr = strerror(errno);
            strcat( reportStr, "[ RTMPERR ][ ");
            strcat( reportStr, _pFuncName );
            strcat( reportStr, " ][ " );
            strcat( reportStr, errorStr );
            strcat( reportStr, " ]");
        } else {
            if ( _pLog ) {
                strcat( reportStr, "[ RTMPHB ][ ");
                strcat( reportStr, _pFuncName );
                strcat( reportStr, " ][ " );
                strcat( reportStr, _pLog );
                strcat( reportStr, " ]");
            }
        }
        ret = Report( gMqttCtx.accountId, gTopic, reportStr, strlen(reportStr) );
        if ( ret != RET_OK ) {
            DBG_LOG("Report error\n");
        }
        *_pStartTime = *_pEndTime;
    } 

    
}

int GetTimeDiff( struct timeval *_pStartTime, struct timeval *_pEndTime )
{
    int time = 0;

    if ( _pEndTime->tv_sec < _pStartTime->tv_sec ) {
        return -1;
    }

    /*DBG_LOG("_pStartTime->tv_sec = %d\n", _pStartTime->tv_sec );*/
    /*DBG_LOG("_pStartTime->tv_usec = %d\n", _pStartTime->tv_usec );*/
    /*DBG_LOG("_pEndTime->tv_usec = %d\n", _pEndTime->tv_usec );*/
    /*DBG_LOG("_pEndTime->tv_sec = %d\n", _pEndTime->tv_sec );*/

    if ( _pEndTime->tv_usec < _pStartTime->tv_usec ) {
        time = (_pEndTime->tv_sec - 1 - _pStartTime->tv_sec) +  
            ((1000000-_pStartTime->tv_usec) + _pEndTime->tv_usec)/1000000;
    } else {
        time = (_pEndTime->tv_sec - _pStartTime->tv_sec) + 
            (_pEndTime->tv_usec - _pStartTime->tv_usec)/1000000;
    }

    return ( time );

}

static int RtmpH264Send(IN char *_pData, IN int _nLen, IN double _dTimeStamp, IN int _nIskey)
{
	RtmpPubContext *pRtmpc = gAppContext.pRtmpc;
	int s32Ret = QN_FAIL;

    {
        char log[64] = {0};
        static struct timeval start = {0,0}, end;  

        itoa( _nLen, log );
        MqttSendRtmpLog( "RtmpH264Send", log, 0, &start, &end );
    }
	
	if (_pData == NULL || pRtmpc == NULL) {
		if (pRtmpc == NULL) {
			QnDemoPrint(DEMO_NOTICE, "%s[%d] Rtmp context is null.\n", __func__, __LINE__);
		} else {
			QnDemoPrint(DEMO_NOTICE, "%s[%d] _pData is NULL.\n", __func__, __LINE__);
			s32Ret = QN_SUCCESS;
		}
		goto RTMPH264SEND_EXIT;
	}
	
	long long int presentationTime = (long long int)_dTimeStamp;
	Nalu *pNalu = NULL;
	int nIdx = 0;
	
	pthread_mutex_lock(&gAppContext.pushLock);
	if (gAppContext.status == RTMP_START) {
		
		const int maxBufSize = 1024 * 1024 * 10;
		char *buf = (char *)malloc(maxBufSize);
		if (!buf) {
			QnDemoPrint(DEMO_WARNING, "%s[%d] malloc failed for buf.\n", __func__, __LINE__);
			goto RTMPH264SEND_UNLOCK;
		}
	
		if (_nIskey && gAppContext.videoState == QN_FALSE) {
			RtmpPubSetVideoTimebase(pRtmpc, presentationTime);
			pNalu = ParseNalu(_pData, _nLen);
			if (!pNalu) {
				goto RTMPH264SEND_FREE_UNLOCK;
			}
			RtmpPubSetSps(pRtmpc, pNalu->data, pNalu->size);
			nIdx += pNalu->size + 4;
			DestroyNalu(&pNalu);

			pNalu = ParseNalu(_pData + nIdx, _nLen - nIdx );
			if (!pNalu) {
				goto RTMPH264SEND_FREE_UNLOCK;
			}
			RtmpPubSetPps(pRtmpc, pNalu->data, pNalu->size);
			nIdx += pNalu->size + 4;
			DestroyNalu(&pNalu);

			gAppContext.videoState = QN_TRUE;
			gAppContext.isOk = QN_TRUE;
		}

		if (_nIskey) {
			/*QnDemoPrint(DEMO_WARNING, "RtmpH264Send ... ...\n");*/
		}

		int bufSize = 0;
		while (nIdx < _nLen) {
			pNalu = ParseNalu(_pData + nIdx, _nLen - nIdx);
			if (!pNalu) {
				QnDemoPrint(DEMO_WARNING, "%s[%d] ParseNalu failed.\n", __func__, __LINE__);
				break;
			}
			
			if (bufSize + pNalu->size > maxBufSize) {
				QnDemoPrint(DEMO_WARNING, "%s[%d] malloc failed for buf\n", __func__, __LINE__);
				DestroyNalu(&pNalu);
				goto RTMPH264SEND_FREE_UNLOCK;
			}
			
			if (((pNalu->data[0] & 0x0f) == 0x01) || ((pNalu->data[0] & 0x0f) == 0x05)) {
				
				if (pNalu->size < 0) {
					QnDemoPrint(DEMO_WARNING, "%s[%d]  nalU->size < 0\n", __func__, __LINE__);
					goto RTMPH264SEND_FREE_UNLOCK;
				}
				memcpy(&buf[bufSize], pNalu->data, pNalu->size);
				bufSize += pNalu->size;
				if (bufSize >= _nLen) {
					QnDemoPrint(DEMO_WARNING, "%s[%d] bufSize >= textLen\n", __func__, __LINE__);
					goto RTMPH264SEND_FREE_UNLOCK;
				}
			}
			nIdx += pNalu->size + 4;
			DestroyNalu(&pNalu);
		}
		
		if (bufSize == 0) {
			QnDemoPrint(DEMO_NOTICE, "%s[%d] bufSize == 0.\n", __func__, __LINE__);
			goto RTMPH264SEND_FREE_UNLOCK;
		}
		
		if (false == RtmpIsConnected(gAppContext.pRtmpc)) {
			QnDemoPrint(DEMO_ERR, "connect error ,reInit Rtmp.\n");
			goto REINIT_RTMP;
		}

		if (_nIskey) {
			if (RtmpPubSendVideoKeyframe(pRtmpc, buf, bufSize, presentationTime) != QN_SUCCESS) {
                // add by liyq
                static struct timeval start = { 0, 0 }, end; 

                MqttSendRtmpLog( "RtmpPubSendVideoKeyframe", NULL, 1, &start, &end );
                DBG_LOG("RtmpPubSendVideoKeyframe() error\n");
				QnDemoPrint(DEMO_ERR, "Send video key frame fail, reInit Rtmp.\n");
				goto REINIT_RTMP;
			}
		}
		else {
			if (RtmpPubSendVideoInterframe(pRtmpc, buf, bufSize, presentationTime) != QN_SUCCESS) {
                // add by liyq
                static struct timeval start = { 0, 0 }, end; 

                MqttSendRtmpLog( "RtmpPubSendVideoInterframe", NULL, 1, &start, &end );
				QnDemoPrint(DEMO_ERR, "Send video inter frame fail, reInit Rtmp.\n");
				goto REINIT_RTMP;
			}
		}

        pthread_mutex_lock( &gMqttMutex );
        gTotalSendBytes += bufSize;
        pthread_mutex_unlock( &gMqttMutex );
		
		s32Ret = QN_SUCCESS;
		goto RTMPH264SEND_FREE_UNLOCK;
		
REINIT_RTMP:
			RtmpReconnect(gAppContext.pRtmpc);

RTMPH264SEND_FREE_UNLOCK:
			if (buf) {
				free(buf);
			}
	}  else {
		s32Ret = QN_SUCCESS;
	}
		
RTMPH264SEND_UNLOCK:
	pthread_mutex_unlock(&gAppContext.pushLock);

RTMPH264SEND_EXIT:
	return s32Ret;
}

static int RtmpAudioSend(IN char *_pData, IN int _nLen, IN double _nTimeStamp, IN unsigned int _uAudioType)
{
	long long int presentationTime = (long long int)_nTimeStamp;
	RtmpPubContext *pRtmpc = gAppContext.pRtmpc;
	static unsigned char audioSpecCfg[] = {0x14, 0x10}; 
	int nRet = QN_FAIL;

    // add by liyq
    {
        char log[64] = {0};
        static struct timeval start = { 0 , 0 }, end; 

        itoa( _nLen, log );
        MqttSendRtmpLog( "RtmpAudioSend", log, 0, &start, &end );
    }

	pthread_mutex_lock(&gAppContext.pushLock);
	if (gAppContext.status == RTMP_START) {
		if (gAppContext.isOk == QN_FALSE) {
			//QnDemoPrint(DEMO_INFO, "%s[%d] wait for video metadata send.\n", __func__, __LINE__);
			usleep(5000);
			goto RTMPAUDIO_UNLOCK;
		}

		if (gAppContext.audioState == QN_FALSE) {
			RtmpPubSetAudioTimebase(pRtmpc, presentationTime);
			if (_uAudioType == AUDIO_TYPE_AAC) {
				RtmpPubSetAac(pRtmpc, audioSpecCfg, sizeof(audioSpecCfg));
			}
			gAppContext.audioState = QN_TRUE;
		}
		
		int uIdx = 0;
		int bufSize = 0;
		int headLen = 0;
		Adts *adts = NULL;
		char *buf = NULL;

		buf = (char *)malloc(_nLen);
		if (buf == NULL) {
			QnDemoPrint(DEMO_WARNING, "%s[%d] malloc buffer failed.\n", __func__, __LINE__);
			goto RTMPAUDIO_UNLOCK;
		}

		while (uIdx < _nLen) {
			
			adts = ParseAdts(_pData + uIdx, _nLen - uIdx);
			if (!adts) {
				goto RTMPAUDIO_FREE_UNLOCK;
			}
			//adts header Ϊ7bytes
			if (GetAdtsFHL(adts, &headLen) == QN_FAIL) {
				QnDemoPrint(DEMO_INFO, "%s[%d]  getAdtsFHL failed.\n", __func__, __LINE__);
				DestroyAdts(&adts);
				goto RTMPAUDIO_FREE_UNLOCK;
			}
			if ((adts->size - headLen) < 0) {
				DestroyAdts(&adts);
				QnDemoPrint(DEMO_INFO, "%s[%d] adts->size - headLen < 0\n", __func__, __LINE__);
				goto RTMPAUDIO_FREE_UNLOCK;
			}
			memcpy(&buf[bufSize], &adts->data[headLen], adts->size - headLen);
			bufSize += adts->size - headLen;
			if (bufSize >= _nLen) {
				QnDemoPrint(DEMO_WARNING, "%s[%d] bufSize >= textLen\n", __func__, __LINE__);
				DestroyAdts(&adts);
			    goto RTMPAUDIO_FREE_UNLOCK;
			}
			uIdx = uIdx + adts->size;
			DestroyAdts(&adts);
		}
		nRet = RtmpPubSendAudioFrame(pRtmpc, buf, bufSize, presentationTime);
        if ( nRet != 0 ) {
            static struct timeval start, end; 

            MqttSendRtmpLog( "RtmpPubSendAudioFrame", NULL, 1, &start, &end );
        }

// add by liyq
        pthread_mutex_lock( &gMqttMutex );
        if ( nRet == 0 ) {
            gTotalSendBytes += bufSize;
        }
        pthread_mutex_unlock( &gMqttMutex );
// add by liyq

		nRet = QN_SUCCESS;
		
RTMPAUDIO_FREE_UNLOCK:
		if (buf) {
			free(buf);
			buf = NULL;
		}
	}
	
RTMPAUDIO_UNLOCK:
	pthread_mutex_unlock(&gAppContext.pushLock);
	
	return nRet;
}

int RtmpInit()
{
	//rtmp sdk上线文去初始化前检查
	if (gAppContext.pRtmpc) {
		QnDemoPrint(DEMO_INFO, "%s[%d] RtmpPubDel release Rtmp context.\n", __func__, __LINE__);
		RtmpPubDel(gAppContext.pRtmpc);
	}
	char RtmpUrl[RTMPURL_LEN] = {0};

	AjGetRtmpUrl(&RtmpUrl[0]);
	QnDemoPrint(DEMO_ERR, "%s[%d] Rtmp url :%s\n", __func__, __LINE__, RtmpUrl);

	//建立rtmp推流连接
	do {
			gAppContext.pRtmpc = RtmpPubNew(RtmpUrl, 10, RTMP_PUB_AUDIO_AAC, RTMP_PUB_AUDIO_AAC, RTMP_PUB_TIMESTAMP_ABSOLUTE);
			if (!gAppContext.pRtmpc) {
				QnDemoPrint(DEMO_ERR, "%s[%d] Get Rtmp context failed.\n", __func__, __LINE__);
				return QN_FAIL;
			}
			if (RtmpPubInit(gAppContext.pRtmpc) != QN_SUCCESS) {
				RtmpPubDel(gAppContext.pRtmpc);
				gAppContext.pRtmpc = NULL;
				continue;
			}
			if (RtmpPubConnect(gAppContext.pRtmpc) != QN_SUCCESS) {
				RtmpPubDel(gAppContext.pRtmpc);
				gAppContext.pRtmpc = NULL;
			}
			else {
				break;
			}
	} while(1);
	pthread_mutex_init(&gAppContext.pushLock, NULL);	
	gAppContext.RtmpH264Send = RtmpH264Send;
	gAppContext.RtmpAudioSend = RtmpAudioSend;
	gAppContext.videoState = QN_FALSE;
	gAppContext.audioState = QN_FALSE;
	gAppContext.isOk = QN_FALSE;
	gAppContext.status = RTMP_START;

	QnDemoPrint(DEMO_WARNING, "%s[%d] Init succeed.\n", __func__, __LINE__);
	return QN_SUCCESS;
}

void RtmpRelease()
{
	RtmpPubDel(gAppContext.pRtmpc);
	gAppContext.pRtmpc = NULL;
	gAppContext.RtmpH264Send = NULL;
	gAppContext.RtmpAudioSend = NULL;
	pthread_mutex_destroy(&gAppContext.pushLock);	
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

int RtmpIsConnected(RtmpPubContext* rpc)
{
	return rpc->m_pRtmp->m_sb.sb_socket != -1;
}

int RtmpReconnect(RtmpPubContext* rpc)
{
	char RtmpUrl[RTMPURL_LEN] = {0};
	AjGetRtmpUrl(&RtmpUrl[0]);
	QnDemoPrint(DEMO_ERR, "%s[%d] Rtmp url :%s\n", __func__, __LINE__, RtmpUrl);

	RtmpPubDel(gAppContext.pRtmpc);

		do {
			gAppContext.pRtmpc = RtmpPubNew(RtmpUrl, 10, RTMP_PUB_AUDIO_AAC, RTMP_PUB_AUDIO_AAC, RTMP_PUB_TIMESTAMP_ABSOLUTE);
			if (!gAppContext.pRtmpc) {
				QnDemoPrint(DEMO_ERR, "%s[%d] Get Rtmp context failed.\n", __func__, __LINE__);
				return QN_FAIL;
			}
			if (RtmpPubInit(gAppContext.pRtmpc) != QN_SUCCESS) {
				RtmpPubDel(gAppContext.pRtmpc);
				gAppContext.pRtmpc = NULL;
				continue;
			}
			if (RtmpPubConnect(gAppContext.pRtmpc) != QN_SUCCESS) {
				RtmpPubDel(gAppContext.pRtmpc);
				gAppContext.pRtmpc = NULL;
			}
			else {
				break;
			}
	} while(1);
	gAppContext.RtmpH264Send = RtmpH264Send;
	gAppContext.RtmpAudioSend = RtmpAudioSend;
	gAppContext.videoState = QN_FALSE;
	gAppContext.audioState = QN_FALSE;
	gAppContext.isOk = QN_FALSE;
	gAppContext.status = RTMP_START;
	QnDemoPrint(DEMO_WARNING, "%s[%d] Reconnect succeed.\n", __func__, __LINE__);
	return QN_SUCCESS;

}

void InitSigHandler(void)
{
	struct sigaction sa = {};
	
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGPIPE);

	sa.sa_handler = SigPipeHandler;
	sigaction(SIGPIPE, &sa, NULL);
	QnDemoPrint(DEMO_WARNING, "%s[%d] Init signal handler.\n", 
		__func__, __LINE__);
}

void RtmpStatus(IN int _nStatus)
{
	pthread_mutex_lock(&gAppContext.pushLock);
	gAppContext.status = _nStatus;
	if (_nStatus == RTMP_START) {
		if (RtmpInit() == QN_FAIL) {
			QnDemoPrint(DEMO_WARNING, "Start Rtmp fail.\n");
		}
		QnDemoPrint(DEMO_WARNING, 
"%s[%d] set RTMP_START status.\n", 
		__func__, __LINE__);
	} else {
		QnDemoPrint(DEMO_WARNING, "%s[%d] set RTMP_STOP status.\n", 
			__func__, __LINE__);
		RtmpRelease();
	}
	pthread_mutex_unlock(&gAppContext.pushLock);
}

void InitMqttMutex()
{
    pthread_mutex_init( &gMqttMutex, 0 );
}

void DestroyMqttMutex()
{
    pthread_mutex_destroy( &gMqttMutex );
}

