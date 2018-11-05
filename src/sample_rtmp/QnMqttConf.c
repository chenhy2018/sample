#include "QnMqttConf.h"


struct MqttDefaultcfg gMqttDefault = {
    .ua = "test",
    .username = "1023",
    .pass = "6epJRKvx",
    .host = "180.97.147.170",
    // .host = "47.105.118.51",
    .topic = "test1",
    .port = 1883,
};

struct MqttDefaultcfg* NewMqttDefaultcfg()
{
    return (struct MqttDefaultcfg *)malloc(sizeof(struct MqttDefaultcfg));
}

void DeleteMqttDefaultcfg(struct MqttDefaultcfg *cfg)
{
    if (cfg) {
        free(cfg);
    }
}

void set_mqttconfig(struct MqttDefaultcfg mqttcfg)
{
    QnDemoPrint(DEMO_ERR, "%s:%d>> ua:%s  username:%s  pass:%s  host:%s  topic:%s    port:%d\n", __func__, __LINE__,
                gMqttDefault.ua, gMqttDefault.username, gMqttDefault.pass, gMqttDefault.host, gMqttDefault.topic, gMqttDefault.port);
    // memcpy(&gMqttDefault.ua[0], &mqttcfg.ua[0], MQTT_ELEMENT_LEN);
    // memcpy(&gMqttDefault.username[0], &mqttcfg.username[0], sizeof(mqttcfg.username));
    // memcpy(&gMqttDefault.pass[0], &mqttcfg.pass[0], MQTT_ELEMENT_LEN);
    // memcpy(&gMqttDefault.host[0], &mqttcfg.host[0], MQTT_ELEMENT_LEN);
    // memcpy(&gMqttDefault.topic[0], &mqttcfg.topic[0], MQTT_ELEMENT_LEN);
    // gMqttDefault.port = mqttcfg.port;

    QnDemoPrint(DEMO_ERR, "%s:%d>> ua:%s  username:%s  pass:%s  host:%s  topic:%s    port:%d\n", __func__, __LINE__,
                gMqttDefault.ua, gMqttDefault.username, gMqttDefault.pass, gMqttDefault.host, gMqttDefault.topic, gMqttDefault.port);
}

static void mqtt_getua(char *ua)
{
    if (ua) {
        memcpy(ua, &gMqttDefault.ua[0], sizeof(gMqttDefault.ua));
    }
}

static  void mqtt_getusername(char *username)
{
    if (username) {
        memcpy(username, gMqttDefault.username, sizeof(gMqttDefault.username));
    }
}

static void mqtt_getpass(char *pass)
{
    if (pass) {
        memcpy(pass, &gMqttDefault.pass[0], sizeof(gMqttDefault.pass));
    }
}

static void mqtt_gethost(char *host)
{
    if (host) {
        memcpy(host, &gMqttDefault.host[0], sizeof(gMqttDefault.host));
    }
}

static void mqtt_gettopic(char *topic)
{
    if (topic) {
        memcpy(topic, &gMqttDefault.topic[0], sizeof(gMqttDefault.topic));
    }
}

static void mqtt_getport(int *port)
{
    if (port) {
        *port = gMqttDefault.port;
    }
}

static int mqtt_getconf(struct Conf *this)
{
    if (!this) {
        return QN_FAIL;
    }
    struct MqttConf *self = container_of(this, struct MqttConf, base);
    mqtt_getua(&self->ua[0]);
    mqtt_getusername(&self->username[0]);
    mqtt_getpass(&self->pass[0]);
    mqtt_gethost(&self->host[0]);
    mqtt_gettopic(&self->topic[0]);
    mqtt_getport(&self->port);
    QnDemoPrint(DEMO_ERR, "%s:%d>> ua:%s  username:%s pass:%s  host:%s  topic:%s  port:%d\n", __func__, __LINE__, self->ua, self->username, 
                    self->pass, self->host, self->topic, self->port);
    if (this->next && this->next->ops && this->next->ops->GetConf) {
        QnDemoPrint(DEMO_ERR, "%s:%d>> nnnnnnnnn\n", __func__, __LINE__);
        this->next->ops->GetConf(this->next);
    }
    
    return QN_SUCCESS;
}

static int mqtt_setconf(struct Conf *this)
{
    return QN_SUCCESS;
}

static const struct ConfOps gMqttConfOps = {
    .GetConf = mqtt_getconf,
    .SetConf = mqtt_setconf,
};

struct MqttConf *NewMqttConf(struct Conf *this)
{
    struct MqttConf *self = (struct MqttConf *)malloc(sizeof(struct MqttConf));
    if (self) {
        memset(self, 0, sizeof(struct MqttConf));
        self->base.next = this;
        self->base.ops = &gMqttConfOps;
    }
    return self;
}

void DeleteMqttConf(struct MqttConf *self)
{
    if (self) {
        free(self);
    }
}

void mqtt_show(struct MqttConf *self)
{
    QnDemoPrint(DEMO_ERR, "ua:%s\n", self->ua);
    QnDemoPrint(DEMO_ERR, "pass:%s\n", self->pass);
    QnDemoPrint(DEMO_ERR, "host:%s\n", self->host);
    QnDemoPrint(DEMO_ERR, "topic:%s\n", self->topic);
}