#ifndef __AJFRAMEHANDLERSDK_H__
#define __AJFRAMEHANDLERSDK_H__

#ifdef __cplusplus
extern "C"  {
#endif

#include "QnFrameHandlerSDK.h"
#include "QnMqttConf.h"
#include "QnRtmpConf.h"
#include "devsdk.h"


#define AJ_ELEMENT_LEN (32)

typedef enum{
    MODULE_VIDEO,
    MODULE_AUDIO,
    MODULE_ALARM
}ModuleType;

typedef enum{
    AUDIO_NONE = 0,
    AUDIO_AAC,
    AUDIO_G711A,
    AUDIO_G711U,
    AUDIO_PCM
};

struct AjIpcConfig
{
    struct MqttDefaultcfg mqtt;
    struct RtmpDefaultcfg rtmp;
};

struct AjFrameHandlerSdk 
{
    struct AjIpcConfig *ipccfg;
    int counter;
    int video_chan;
    int video_id;
    int audio_chan;
    int audio_id;
    int (*video_callback)(char *frame, int len, double timestamp, int iskey);
    int (*audio_callback)(char *frame, int len, double timestamp, unsigned int type);
    int (*alarm_callback)(ALARM_ENTRY alarm, void *context);
    struct FrameHandlerSdk base;
};

struct AjFrameHandlerSdk* NewAjFrameHandlerSdk();
void DeleteAjFrameHandlerSdk(struct AjFrameHandlerSdk *ajsdk);

#ifdef __cplusplus
}
#endif

#endif