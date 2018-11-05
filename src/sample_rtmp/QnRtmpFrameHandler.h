#ifndef __QNRTMPFRAMEHANDLER_H__
#define __QNRTMPFRAMEHANDLE R_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "QnFrameHandler.h"
#include "rtmp_publish.h"
#include "QnRtmpConf.h"

#define RTMP_START (1)
#define RTMP_STOP (0)

#define AUDIO_TYPE_G711 (0)
#define AUDIO_TYPE_AAC (1)

struct RtmpFrameHandler
{
    RtmpPubContext *pRtmpCtx;
    int counter;
    char *url;
    int videoState;
    int audioState;
    int isOk;
    int state;
    struct FrameHandler base;
    struct Conn *conn;
    pthread_mutex_t rtmplock;
};

typedef struct 
{
    int size;
    char *data;
}Nalu;

typedef struct 
{
    unsigned int size;
    unsigned char *data;
}Adts;

struct RtmpFrameHandler *NewRtmpFrameHandler(struct RtmpConf *cfg);
void DeleteRtmpFrameHandler(struct RtmpFrameHandler *handler);

#ifdef __cplusplus
}
#endif

#endif