//
// Created by chenh on 2018/7/20.
//

#ifndef __QNMQTTCONN_H__
#define __QNMQTTCONN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "QnCommon.h"
#include "QnConn.h"
#include "QnMqttConf.h"

#define MQTT_ELEMENT_LEN 64
#define MQTT_MAX 1

struct ConnectStatus
{
    int status;
    void *instance;
    char topic[MQTT_ELEMENT_LEN];
};

struct MqttConn
{
    unsigned int counter;
    struct Conn base;
 //   struct MqttOption opt;
    
    struct ConnectStatus status[MQTT_MAX];//int account;
};


struct MqttConn *NewMqttConn();
void DeleteMqttConn(struct MqttConn *self);

#ifdef __cplusplus
}
#endif
#endif 
