#include <pthread.h>
#include <string.h>
#include "QnMqttConn.h"
#include "QnMqttConf.h"
#include "mqtt.h"

#define CONNECT_MAX (10)

//一个ipc 一个mqtt instance，pub和sub共用一个instance
static struct MqttConn *mqttconn = NULL;

void OnMessage(const void *instance, int accountid, const char *topic, const char *message, size_t length)
{
    QnDemoPrint(DEMO_ERR, "%s:%d>> %p topic %s message %s\n", __func__, __LINE__, instance, topic, message);
}

void OnEvent(const void *instance, int accountid, const id, const char *reason)
{
    QnDemoPrint(DEMO_ERR, "%s:%d>> %p id= %d, reason= %s\n", __func__, __LINE__, instance, id, reason);
    int i = 0;

    if (mqttconn) {
        struct ConnectStatus *status;

        for (; i < 10; i++) {
            if (mqttconn->status[i].instance == instance) {
                status = &mqttconn->status[i];
            }
        }
        status->status = id;
    }
}

static void mqtt_write(struct Conn *this, const char *msg, int len)
{
    struct MqttConn *conn = container_of(this, struct MqttConn, base);

    if (msg) {
        QnDemoPrint(DEMO_INFO, "==========mqtt_write========\n");
        MqttPublish(conn->status[0].instance, &conn->status[0].topic[0], len, msg);
    }
}

static void mqtt_start(struct Conn *this)
{
    struct MqttConn *conn = container_of(this, struct MqttConn, base);

    MqttSubscribe(conn->status[0].instance, &conn->status[0].topic[0]);

}

static void mqtt_stop(struct Conn *this)
{
    struct MqttConn *conn = container_of(this, struct MqttConn, base);
    MqttUnsubscribe(conn->status[0].instance, &conn->status[0].topic[0]);   
}

static void mqtt_init()
{
    MqttLibInit();
}

static void mqtt_exit()
{
    MqttLibCleanup();
}

static const struct ConnOps gMqttConnOps  = {
    .ConnWrite = mqtt_write,
    // .ConnRead = mqtt_read,
    // .ConnOpen = mqtt_open,
    .ConnInit = mqtt_init,
    .ConnExit = mqtt_exit,
};

struct MqttConn *NewMqttConn()
{
    struct MqttOptions opt;
    struct MqttConf *mqttcfg = NULL;

    if (!mqttconn) {
        mqttcfg = NewMqttConf(NULL);
        if (!mqttcfg) {
            QnDemoPrint(DEMO_ERR, "%s:%d>> Create Mqtt connect fail.\n", __func__, __LINE__);
            return NULL;
        }
        mqttcfg->base.ops->GetConf(&mqttcfg->base);
        mqttconn = (struct MqttConn *)malloc(sizeof(struct MqttConn));
        if (mqttconn) {
            mqtt_init();

            memset(mqttconn, 0, sizeof(struct MqttConn));
            mqttconn->base.ops = &gMqttConnOps;
            QnDemoPrint(DEMO_INFO, "%s:%d>> mqttcfg, ua: %s  username :%s  pass:%s  host:%s  port:%d topic:%s\n", __func__, __LINE__,\
                            mqttcfg->ua, mqttcfg->username, mqttcfg->pass, mqttcfg->host, mqttcfg->port, mqttcfg->topic);
            
            if (mqttcfg) {
                memcpy(&mqttconn->status[0].topic[0], &mqttcfg->topic[0], sizeof(mqttconn->status[0].topic));
                opt.pId = &mqttcfg->ua[0];
                opt.primaryUserInfo.pUsername = mqttcfg->username;
                opt.primaryUserInfo.pPassword = mqttcfg->pass;
                opt.primaryUserInfo.pHostname = mqttcfg->host;
                opt.primaryUserInfo.nPort = mqttcfg->port;
            }
            opt.bCleanSession = false;
            opt.nKeepalive = 10;
            opt.nQos = 0;
            opt.bRetain = false;
            opt.callbacks.OnMessage = &OnMessage;
            opt.callbacks.OnEvent = &OnEvent;
            opt.primaryUserInfo.nAuthenicatinMode = MQTT_AUTHENTICATION_USER;
            opt.primaryUserInfo.pCafile = NULL;
            opt.primaryUserInfo.pCertfile = NULL;
            opt.primaryUserInfo.pKeyfile = NULL;

            mqttconn->status[0].instance = MqttCreateInstance(&opt);
            while (!mqttconn->status[0].status & MQTT_CONNECT_SUCCESS) {
                sleep(1);
            }
        }
        DeleteMqttConf(mqttcfg);
    }
    if (mqttconn) {
        mqttconn->counter++;
    }

    return mqttconn;
}

void DeleteMqttConn(struct MqttConn *mqttconn)
{
    mqttconn->counter--;
    if (mqttconn->counter == 0) {
        mqtt_exit();
        free(mqttconn);
        mqttconn = NULL;
        
    }
}

void TestMqttConnect(void *mqttConf)
{
    struct MqttConn *conn = NewMqttConn();
    if (conn) {
        conn->base.ops->ConnWrite(&conn->base, "hello", sizeof("hello"));
    }
}