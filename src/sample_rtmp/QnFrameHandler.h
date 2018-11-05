#ifndef __QNFRAMEHANDLER_H__
#define __QNFRAMEHANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "QnFrameHandlerSDK.h"
#include "QnMsg.h"

struct FrameHandlerOperations 
{
    void (*Open)(struct FrameHandler *handler);
    void (*Close)(struct FrameHandler *handler);
    int (*SendVideo)(char *frame, int len, double timestamp, int iskey);
    int (*SendAudio)(char *frame, int len, double timestamp);
};

struct FrameHandler{
    struct Conf *cfg;
    struct Msg *msg;
    struct FrameHandlerOperations *ops;
    struct FrameHandlerSdk *sdk;
    void *ctx;
};

#ifdef __cplusplus
}
#endif

#endif