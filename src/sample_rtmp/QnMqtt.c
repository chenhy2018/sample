//
// Created by chenh on 2018/7/20.
//
#include <pthread.h>
#include <sys/time.h>
#include "QnCommon.h"
#include "QnConf.h"
#include "QnMqttConf.h"
#include "QnMqtt.h"



QnMqttRegisterContext gMqttCtx;
static int gMqttState = 1;
int gMqttAlreadyConnecgted = 0;

struct mqttRuntime gMqttRt = {};

Event *NewEvent()
{
    Event *pEvent =  (Event *)malloc(sizeof(Event));
    return pEvent;
}
void DelEvent(Event *event)
{
    if (event) {
        free(event);
    }
}

int qn_sdk_init(struct MqttConf *mqttCfg)
{
    if (!mqttCfg) {
        return QN_FAIL;
    }

    if (InitSDK(NULL, 0) != QN_SUCCESS) {
        QnDemoPrint(DEMO_WARNINIG, "%s[%d] Init Qn SDK fail.\n", __func__, __LINE__);
        return QN_FAIL;
    } 
    gMqttRt.account = Register(&mqttCfg->ua[0], &mqttCfg->pass[0], NULL, &mqttCfg->host[0]);
    return QN_SUCCESS;
}

void qn_sdk_uninit()
{
    UninitSDK();
}

static void *mqttPubHandler(void *param)
{

}

static void *mqttSubHandler(void *param)
{
    ErrorID ret = 0;
    struct mqttRuntime *mqttRt = (struct mqttRuntime *)param;
    Event *mqttEvent;

    while (1) {
        mqttEvent = NewEvent();
        if (mqttEvent) {
            break;
        }
    }   

    while (1) {

        ret = PollEvent(mqttRt->id, &mqttEvent->type, &mqttEvent, 100);
        if (ret >= RET_MEM_ERROR) {
            QnDemoPrint(DEMO_ERR, "%s[%d]PollEvent error, ret = %d\n", __func__, __LINE__, ret);
            return NULL;
        }
        if (ret == RET_RETRY) {
            continue;
        }

       switch (mqttEvent->type) {
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
                break;
            }
           case EVENT_MESSAGE:
            {
                break;
            }
           default:
            {
                break;
            }
       }
    }

}
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
        strcat(log, "001 "); 
        strcat( log, "MQHB "); 
       itoa( gTotalSendBytes, strTotalBytes );
        strcat( log, strTotalBytes );
        strcat( log, " ");
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

void mqtt_start()
{
    pthread_t mqttPTr, mqttSTr;
    pthread_attr_t mqttPTrAttr;
    pthread_attr_init(&mqttPTrAttr);
    pthread_attr_serdetachstate(&mqttPTrAttr, PTHREAD_CREATE_DETACHED);

    pthread_create(&mqttPTr, &mqttPTrAttr, mqttPubHandler, NULL);
    pthread_create(&mqttSTr, &mqttPTrAttr, mqttSubHandler, NULL);
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


