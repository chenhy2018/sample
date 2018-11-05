#ifndef __QNMQTTCONF_H__
#define __QNMQTTCONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "QnCommon.h"
#include "QnConf.h"

#define MQTT_ELEMENT_LEN 64

struct MqttDefaultcfg
{
    char ua[MQTT_ELEMENT_LEN];
    char username[MQTT_ELEMENT_LEN];
    char pass[MQTT_ELEMENT_LEN];
    char host[MQTT_ELEMENT_LEN];
    char topic[MQTT_ELEMENT_LEN];
    int port;
};

struct MqttConf 
{
    char ua[MQTT_ELEMENT_LEN];
    char username[MQTT_ELEMENT_LEN];
    char pass[MQTT_ELEMENT_LEN];
    char host[MQTT_ELEMENT_LEN];
    char topic[MQTT_ELEMENT_LEN];
    int port;
    struct Conf base;
};

struct MqttConf *NewMqttConf(struct Conf *this);
void DeleteMqttConf(struct MqttConf *self);
void mqtt_show(struct MqttConf *self);
void set_mqttconfig(struct MqttDefaultcfg cfg);
struct MqttDefaultcfg *NewMqttDefaultcfg();
void DeleteMqttDefaultcfg(struct MqttDefaultcfg *cfg);

#ifdef __cplusplus
}
#endif
#endif