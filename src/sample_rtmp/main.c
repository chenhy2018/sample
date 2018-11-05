#include <stdio.h>
#include <stdlib.h>
#include "QnCommon.h"
// #include "QnConf.h"
// #include "QnSysConf.h"
// #include "QnMqttConf.h"
// #include "QnRtmpConf.h"
// #include "QnMqttConn.h"
#include "AjFrameHandlerSDK.h"
#include "QnRtmpFrameHandler.h"
#include "QnBeat.h"

// struct SampleCfg
// {
//     struct SysConf *syscfg;;
//     struct MqttConf *mqttcfg;
//     struct RtmpConf *rtmpcfg;
// }gSampleCfg;

// static int cfg_init(void)
// {
//     gSampleCfg.syscfg = NewSysConf(NULL);
//     if (gSampleCfg.syscfg) {
//         QnDemoPrint(DEMO_ERR, "New SysConf fail.\n");
//         return QN_FAIL;       
//     }
//     gSampleCfg.mqttcfg = NewMqttConf(&gSampleCfg.syscfg->base);
//     if (gSampleCfg.mqttcfg) {
//         QnDemoPrint(DEMO_ERR, "New MqttConf fail.\n");
//         goto RELEASE_SYSCONF;
//     }
//     gSampleCfg.rtmpcfg = NewRtmpConf(&gSampleCfg.mqttcfg->base);
//     if (gSampleCfg.rtmpcfg) {
//         QnDemoPrint(DEMO_ERR, "New RtmpConf fail.\n");
//         goto RELEASE_MQTTCONF;
//     }
//     gSampleCfg.rtmpcfg->base.ops->GetConf(&gSampleCfg.rtmpcfg->base);
//     return QN_SUCCESS;

// RELEASE_MQTTCONF:
//     DeleteMqttConf(gSampleCfg.mqttcfg);
// RELEASE_SYSCONF:
//     DeleteSysConf(gSampleCfg.syscfg);
//     return QN_FAIL;
// }

// void cfg_uninit(void)
// {
//     DeleteRtmpConf(gSampleCfg.rtmpcfg);
//     DeleteMqttConf(gSampleCfg.mqttcfg);
//     DeleteSysConf(gSampleCfg.syscfg);
// }


int main(void)
{
    int ret = QN_FAIL;

    QnDemoPrint(DEMO_ERR, "%s:%d>> ====\n", __func__, __LINE__);
    //配置数据的数据源
    struct AjFrameHandlerSdk *ajsdk = NewAjFrameHandlerSdk();
    if (!ajsdk) {
        QnDemoPrint(DEMO_ERR, "%s:%d>>> Create AjFrameHandlerSdk fail.\n", __func__, __LINE__);
        return QN_FAIL;
    } 
    ajsdk->base.ops->Init(&ajsdk->base);
    struct RtmpConf *rtmpcfg = NewRtmpConf(NULL);
    if (!rtmpcfg) {
        QnDemoPrint(DEMO_ERR, "%s:%d>> Create RtmpConf fail.\n", __func__, __LINE__);
        goto RELEASE_AJFRAMEHANDLERSDK;
    }
    rtmpcfg->base.ops->GetConf(&rtmpcfg->base);
    QnDemoPrint(DEMO_ERR, "%s:%d>> rtmpcfg->url:%s\n", __func__, __LINE__, &rtmpcfg->url[0]);
    //注册媒体数据发送句柄
    struct RtmpFrameHandler *rtmphandler = NewRtmpFrameHandler(rtmpcfg);
    if (!rtmphandler) {
        QnDemoPrint(DEMO_ERR, "%s:%d>> Create RtmpFrameHandler fail.\n", __func__, __LINE__);
        goto RELEASE_RTMPCONF;
    }
    ajsdk->base.ops->Register(&ajsdk->base, 0, rtmphandler->base.ops->SendVideo);   //注册rtmp发送函数
    ajsdk->base.ops->Register(&ajsdk->base, 1, rtmphandler->base.ops->SendAudio);
    //打开媒体数据源
    ajsdk->base.ops->Start(&ajsdk->base);



    //开启心跳
    BeatInit();
    BeatStart();
    for (;;) {
        usleep(100);
    }
    BeatStop();
    ajsdk->base.ops->Stop(&ajsdk->base);

    DeleteRtmpHandler(rtmphandler);
RELEASE_RTMPCONF:
    ajsdk->base.ops->Release(&ajsdk->base);
    DeleteRtmpConf(rtmpcfg);
RELEASE_AJFRAMEHANDLERSDK:
    DeleteAjFrameHandlerSdk(ajsdk);

    return QN_SUCCESS;
}