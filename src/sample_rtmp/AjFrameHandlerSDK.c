#include <stdio.h>
#include <stdlib.h>
#include "QnCommon.h"
#include "AjFrameHandlerSDK.h"
// #include "QnMqttConf.h"

static struct AjFrameHandlerSdk *ajsdk = NULL;

static int CheckAudioEnable()
{
    AudioConfig audioConfig;

    dev_sdk_get_AudioConfig(&audioConfig);
    if (audioConfig.audioEncode.enable != 1) {
        return 0;
    } else {
        return 1;
    }
}

static void aj_get_ua(MediaStreamConfig cfg, char *ua)
{
    memcpy(ua, &cfg.rtmpConfig.streamid, AJ_ELEMENT_LEN);
}

static void aj_get_pass(MediaStreamConfig cfg, char *pass)
{
    memcpy(pass, &cfg.rtmpConfig.server, sizeof(cfg.rtmpConfig.server));
}

static void aj_get_rtmpurl(MediaStreamConfig cfg, char *rtmpurl)
{
    memcpy(rtmpurl, &cfg.rtmpConfig.appname, sizeof(cfg.rtmpConfig.appname));
    QnDemoPrint(DEMO_ERR, "%s:%d>> rtmpurl :%s\n", __func__, __LINE__, rtmpurl);
}

static int get_media_stream_config(MediaStreamConfig *mediacfg)
{
    if (!mediacfg) {
        QnDemoPrint(DEMO_ERR, "%s:%d>> param is null.\n", __func__, __LINE__);
        return QN_FAIL;
    }
    if (dev_sdk_get_MediaStreamConfig(mediacfg) == QN_SUCCESS) {
        return QN_SUCCESS;
    }
    return QN_FAIL;
}

struct AjIpcConfig *NewAjIpcConfig()
{
    QnDemoPrint(DEMO_INFO, "%s:%d\n", __func__, __LINE__);
    return (struct AjIpcConfig *)malloc(sizeof(struct AjIpcConfig));
}

void DeleteAjIpcConfig(struct AjIpcConfig *cfg)
{
    QnDemoPrint(DEMO_INFO, "%s:%d\n", __func__, __LINE__);
    free(cfg);
}

static int aj_get_config(struct FrameHandlerSdk *sdk)
{
    struct AjFrameHandlerSdk *ajsdk = container_of(sdk, struct AjFrameHandlerSdk, base);
    MediaStreamConfig mediacfg = {};
    get_media_stream_config(&mediacfg);
    ajsdk->ipccfg = NewAjIpcConfig();
    if (ajsdk->ipccfg) {
        aj_get_ua(mediacfg, &ajsdk->ipccfg->mqtt.ua[0]);
        aj_get_pass(mediacfg, &ajsdk->ipccfg->mqtt.pass[0]);
        aj_get_rtmpurl(mediacfg, &ajsdk->ipccfg->rtmp.url[0]);
        QnDemoPrint(DEMO_ERR, "%s:%d>> mqtt config , ua:%s  pass:%s\n", __func__, __LINE__, ajsdk->ipccfg->mqtt.ua, ajsdk->ipccfg->mqtt.pass);
        set_mqttconfig(ajsdk->ipccfg->mqtt);
        set_rtmpconfig(ajsdk->ipccfg->rtmp);
        DeleteAjIpcConfig(ajsdk->ipccfg);
        QnDemoPrint(DEMO_ERR, "%s:%d>> \n", __func__, __LINE__);
        return QN_SUCCESS;
    }
    return QN_FAIL;
}

int aj_video_callback(int streamid, char *frame, int len, int iskey, double timestamp, unsigned long frameidx, unsigned long keyframeidx, void *context)
{
    struct FrameHandlerSdk *sdk = (struct FrameHandlerSdk *)context;
    struct AjFrameHandlerSdk *ajsdk = container_of(sdk, struct AjFrameHandlerSdk, base);
    // QnDemoPrint(DEMO_ERR, "%s:%d>>=== video_chan :%d video_id :%d\n", __func__, __LINE__, ajsdk->video_chan, ajsdk->video_id);
    static unsigned int nLastTimeStamp = 0;
    int ret = QN_FAIL;

    if (ajsdk->video_callback == NULL) {
        if (iskey)
            QnDemoPrint(DEMO_ERR, "%s:%d>> ajsdk->base.ops->video_callback is null.\n", __func__, __LINE__);
            return QN_FAIL;
    }
    ret = ajsdk->video_callback(frame, len, timestamp, iskey);
    if (ret != QN_FAIL) {
        return ret;
    }
    return QN_FAIL;
}

static int aj_audio_callback(char *frame, int len, double timestamp, \
    unsigned long frameidx, void *context) 
{
    struct FrameHandlerSdk *sdk = (struct FrameHandlerSdk *)context;
    struct AjFrameHandlerSdk *ajsdk = container_of(sdk, struct AjFrameHandlerSdk, base);
    int ret = QN_FAIL;
    
    if (ajsdk->audio_callback == NULL) {
        QnDemoPrint(DEMO_ERR, "%s:%d>> audio_callback is null.\n", __func__, __LINE__);
        return QN_FAIL;
    }
    ret = ajsdk->audio_callback(frame, len, timestamp, AUDIO_AAC);
    if (ret != QN_FAIL) {
        return ret;
    }
    return QN_FAIL;
}

static void aj_init(struct FrameHandlerSdk *sdk)
{
    struct MqttDefaultcfg *mqttcfg = NULL;
    int ret = QN_SUCCESS;

    struct AjFrameHandlerSdk *ajsdk = container_of(sdk, struct AjFrameHandlerSdk, base);
    ret = dev_sdk_init(DEV_SDK_PROCESS_APP);
    aj_get_config(sdk);
    QnDemoPrint(DEMO_ERR, "%s:%d>> ret = %d\n", __func__, __LINE__, ret);
}

static void aj_release(struct FrameHandlerSdk *sdk)
{
    dev_sdk_release();
    QnDemoPrint(DEMO_ERR, "%s:%d>> \n", __func__, __LINE__);
}

static void aj_start(struct FrameHandlerSdk *sdk)
{
    struct AjFrameHandlerSdk *ajsdk = container_of(sdk, struct AjFrameHandlerSdk, base);
    static int context = 1;
    
    QnDemoPrint(DEMO_INFO, "%s:%d>> channel:%d id:%d\n", \
            __func__, __LINE__, ajsdk->video_chan, ajsdk->video_id);
    dev_sdk_start_video(ajsdk->video_chan, ajsdk->video_id, aj_video_callback, (void *)sdk);
    if (CheckAudioEnable()) {
        dev_sdk_start_audio(ajsdk->audio_chan, ajsdk->audio_id, aj_audio_callback, (void *)sdk);
    } 
    QnDemoPrint(DEMO_INFO, "%s:%d>>\n", __func__, __LINE__);
}

static void aj_stop(struct FrameHandlerSdk *sdk) 
{
    struct AjFrameHandlerSdk *ajsdk = container_of(sdk, struct AjFrameHandlerSdk, base);
    
    dev_sdk_stop_video(ajsdk->video_chan, ajsdk->video_id);
    if (CheckAudioEnable()) {
        dev_sdk_stop_audio(ajsdk->audio_chan, ajsdk->audio_id);
    }
    QnDemoPrint(DEMO_ERR, "%s:%d>>\n", __func__, __LINE__);
}


static void aj_register_callback(struct FrameHandlerSdk *sdk, int module, void *callback)
{
    struct AjFrameHandlerSdk *ajsdk = container_of(sdk, struct AjFrameHandlerSdk, base);
    
    switch (module) {
        case MODULE_VIDEO: {
                ajsdk->video_callback = callback;
            break;
        }

        case MODULE_AUDIO: {
                ajsdk->audio_callback = callback;
            break;
        }

        case MODULE_ALARM: {
                ajsdk->alarm_callback = callback;
            break;
        }
    }
    QnDemoPrint(DEMO_ERR, "%s:%d>> \n", __func__, __LINE__);
}

static aj_unregister_callback(struct FrameHandlerSdk *sdk, int module)
{
    struct AjFrameHandlerSdk *ajsdk = container_of(sdk, struct AjFrameHandlerSdk, base);

    switch (module) {
        case MODULE_VIDEO: {
            ajsdk->video_callback = NULL;
            break;
        }

        case MODULE_AUDIO: {
            ajsdk->audio_callback = NULL;
            break;
        }

        case MODULE_ALARM: {
            ajsdk->alarm_callback = NULL;
            break;
        }
    }
}

static const struct FrameHandlerSdkOperation gAjFrameHandlerSdkOps = {
    .Init = aj_init,
    .Release = aj_release,
    .Start = aj_start,
    .Stop = aj_stop,
    .Register = aj_register_callback,
    .UnRegister = aj_unregister_callback,
};

struct AjFrameHandlerSdk *NewAjFrameHandlerSdk()
{
    if (!ajsdk) {
        ajsdk = (struct AjFrameHandlerSdk *)malloc(sizeof(struct AjFrameHandlerSdk));
        if (ajsdk) {
            ajsdk->base.ops = &gAjFrameHandlerSdkOps;
            ajsdk->video_chan = 0;
            ajsdk->video_id = 0;
            ajsdk->audio_chan = 0;
            ajsdk->audio_id = 1;
            ajsdk->counter = 0;
            ajsdk->video_callback = NULL;
            ajsdk->audio_callback = NULL;
        } else {
            return ajsdk;
        }
    }
    QnDemoPrint(DEMO_INFO, "%s:%d>> ajsdk->counter:%d.\n", __func__, __LINE__, ajsdk->counter);
    ajsdk->counter++;
    return ajsdk;
}

void DeleteAjFrameHandlerSdk(struct AjFrameHandlerSdk *ajsdk)
{
    ajsdk->counter--;
    if (ajsdk->counter) {
        return;
    }
    free(ajsdk);
    ajsdk = NULL;
    QnDemoPrint(DEMO_INFO, "%s:%d>>\n", __func__, __LINE__);
}