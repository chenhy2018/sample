#ifndef __MQTTINTERNAL__
#define __MQTTINTERNAL__


#include <mosquitto.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define STATUS_IDLE 0
#define STATUS_CONNECTING 1
#define STATUS_CONNACK_RECVD 2
#define STATUS_WAITING 3
#define STATUS_CONNECT_ERROR 4
#define MAX_MQTT_TOPIC_SIZE 128

typedef struct Node
{
        char topic[MAX_MQTT_TOPIC_SIZE];
        struct Node *pNext;
}Node;

struct MqttInstance
{
        struct mosquitto *mosq;
        struct MqttOptions options;
        int status;
        int lastStatus;
        bool connected;
        bool isDestroying;
        Node pSubsribeList;
        pthread_mutex_t listMutex;
};

#endif
