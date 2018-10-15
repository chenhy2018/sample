//
// Created by chenh on 2018/7/20.
//
#include <pthread.h>
#include "sdk_interface.h"
#include "QnCommon.h"
#include "QnMqtt.h"
#include <sys/time.h>
#include "ajSdk.h"
#include "QnMqtt.h"
#include "QnRtmp.h"

#define HOST "180.97.147.170"

QnMqttRegisterContext gMqttCtx;
static int gMqttState = 1;
int gMqttAlreadyConnecgted = 0;
char gTopic[256] = "test1";

Event *NewEvent()
{
    return (Event *)malloc(sizeof(Event));
}

void DeleteEvent(IN Event *_pEvent)
{
    if (_pEvent) {
        free(_pEvent);
    }
}

//Qn mqtt 建立连接，并注册账号
int QnMqttInit()
{
    char id[256] = { 0 };
    char passwd[64] = { 0 };
    char *host = HOST;

    //获取mqtt连接建立用到的id和pss
    GetStremId( id );
    GetServerAddr( passwd );
    DBG_LOG("passwd = %s\n", passwd );
    
    //初始化 mqtt sdk
    if (InitSDK(NULL, 0) != RET_OK) {
        QnDemoPrint(DEMO_WARNING, "%s[%d] Init Qn SDK failed.\n", __func__, __LINE__);
        return QN_FAIL;
    }

    //账号注册
    gMqttCtx.accountId = Register( id, passwd, NULL, host );
    usleep(600);

    return QN_SUCCESS;
}

void QnMqttRelease()
{
    free(gMqttCtx.id);
    free(gMqttCtx.password);
    free(gMqttCtx.sigHost);
    free(gMqttCtx.mediaHost);
    free(gMqttCtx.imHost);
    UninitSDK();
    QnDemoPrint(DEMO_INFO, "QnMqttRelease.......\n");
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

//mqtt 事务线程
static void *MqttHandler(void *param)
{
    EventType type;
    Event *pEvent = (Event *)malloc(sizeof(Event));
    ErrorID ret = 0;
    static struct timeval lastTime, now; 
    int interval = 0;
    char strTotalBytes[64] = { 0 };
    unsigned char eventloop = 1;
    char log[64] = { 0 };

    //获取topic
   // GetStremId( gTopic );
    DBG_LOG("topic = %s\n", gTopic );

    while ( eventloop ) {
        //轮询获取Event
        ret = PollEvent( gMqttCtx.accountId, &type, &pEvent, 100);
        if ( ret >= RET_MEM_ERROR ) {
            DBG_LOG("PollEvent error, ret = %d\n", ret );
            return NULL;
        }
        //获取message超时,即没收到消息
        if ( ret == RET_RETRY ) {
            continue;
        }

        //轮询到消息时
        switch (type) {
        case EVENT_CALL:
            {
                CallEvent *pCallEvent = &(pEvent->body.callEvent);
                if (pCallEvent->status == CALL_STATUS_INCOMING) {
                    AnswerCall(gMqttCtx.accountId, pCallEvent->callID);
                }
                if (pCallEvent->status == CALL_STATUS_ERROR
                    || pCallEvent->status == CALL_STATUS_HANGUP) {
                    QnDemoPrint(DEMO_INFO, "%s[%d] CallEvent :CALL_STATUS_ERROR\n", __func__, __LINE__);
                }
                if (pCallEvent->status == CALL_STATUS_ESTABLISHED) {
                    QnDemoPrint(DEMO_INFO, "%s[%d] CallEvent “:CALL_STATUS_ESTABLISHED\n", __func__, __LINE__);
                }
            }
            break;

        //收到消息
        case EVENT_MESSAGE:
            {
                MessageEvent *pMessage = &(pEvent->body.messageEvent);
                if (pMessage->status == MESSAGE_STATUS_CONNECT) {
                    DBG_LOG("get message MESSAGE_STATUS_CONNECT\n");
                    eventloop = 0;
                } else if (pMessage->status == MESSAGE_STATUS_DATA) {
                    QnDemoPrint(DEMO_INFO, "%s[%d] ###MESSAGE_STATUS_DATA###\n", __func__, __LINE__);
                    QnDemoPrint(DEMO_INFO, "Message %s status id :%d account :%d\n",
                                pMessage->message, pMessage->status, gMqttCtx.id);
                } else if (pMessage->status == MESSAGE_STATUS_DISCONNECT) {
                    QnDemoPrint(DEMO_INFO, "###MESSAGE_STATUS_DISCONNECT###\n");
                }
            }
            break;
        }
    }

    //发布消息
    strcpy( log, "[ MQMSG ] [ Power on mqtt thread started !!! ] ");
    ret = Report( gMqttCtx.accountId, gTopic, log,  strlen(log) );
    if ( ret != RET_OK ) {
        DBG_LOG("Report error\n");
    }
    
    //异步发送监控数据
    gMqttAlreadyConnecgted = 1;
    DBG_LOG("start to report mqtt event\n");
    for (;;) {
        pthread_mutex_lock( &gMqttMutex );
        memset( log, 0, sizeof(log) );
        memset( strTotalBytes, 0, sizeof(strTotalBytes) );
        strcat( log, "[ MQHB ][ ");
        itoa( gTotalSendBytes, strTotalBytes );
        strcat( log, strTotalBytes );
        strcat( log, " ]");
        pthread_mutex_unlock( &gMqttMutex );
        DBG_LOG("strTotalBytes = %s\n", strTotalBytes);
        ret =  Report( gMqttCtx.accountId, gTopic, log,  strlen(log) );
        if ( ret != RET_OK ) {
            DBG_LOG("Report error\n");
        }
        pthread_mutex_lock( &gMqttMutex );
        gTotalSendBytes = 0;
        pthread_mutex_unlock( &gMqttMutex );
        sleep(5);
    }

    return NULL;
}

//创建mqtt pub线程
void QnMqttStart()
{
    pthread_t mqttPth;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    gMqttCtx.sendFlag = 0;
    gMqttCtx.timeCount = 0;
    gMqttCtx.callId = 0;
    pthread_create(&mqttPth, &attr, MqttHandler, (void *)&gMqttCtx);
}

