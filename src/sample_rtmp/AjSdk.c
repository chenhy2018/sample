#include <stdio.h>
#include <stdlib.h>
#include "devsdk.h"
#include "QnCommon.h"
#include "QnRtmp.h"
#include "ajSdk.h"


#define VIDEO_CHN_ID 0
#define VIDEO_STREAM_ID 0
#define AUDIO_CHN_ID 0
#define AUDIO_STREAM_ID 1


static MediaStreamConfig gAjMediaStreamConfig = {0};

static int GetMediaStreamConfig(IN MediaStreamConfig *_pMediaStreamConfig);

//初始化Aj sdk
void AjInit()
{
	dev_sdk_init(DEV_SDK_PROCESS_APP);
	GetMediaStreamConfig(&gAjMediaStreamConfig);
}

//开启Aj sdk，数据采集
void AjStart()
{
	static int context = 1;

	dev_sdk_start_video(VIDEO_CHN_ID, VIDEO_STREAM_ID, VideoCallBack, &context);
	if (CheckAudioEnable()) {
		dev_sdk_start_audio(AUDIO_CHN_ID, AUDIO_STREAM_ID, AudioCallBack, &context);
	}
}

void AjStop()
{
	dev_sdk_stop_video(VIDEO_CHN_ID, VIDEO_STREAM_ID);
	if (CheckAudioEnable()) {
		dev_sdk_stop_audio(AUDIO_CHN_ID, AUDIO_STREAM_ID);
	}
}

int AjRelease()
{
	dev_sdk_release();
}

//音频使能判断
int CheckAudioEnable()
{
	AudioConfig audioConfig;

	dev_sdk_get_AudioConfig(&audioConfig);
	if( audioConfig.audioEncode.enable != 1)
		return 0;
	else
		return 1 ;
}

//Aj 获取媒体流配置参数
static int GetMediaStreamConfig(IN MediaStreamConfig *_pMediaStreamConfig)
{
	if (!_pMediaStreamConfig) {
		QnDemoPrint(DEMO_WARNING, "%s[%d] null pointer.\n", __func__, __LINE__);
		return QN_FAIL;
	}
	if (dev_sdk_get_MediaStreamConfig(_pMediaStreamConfig) == QN_SUCCESS) {
		return QN_SUCCESS;
	}

	return QN_FAIL;
}

//从Aj配置获取rtmp url
void AjGetRtmpUrl(IN char *_pUrl)
{
	if (!_pUrl) {
		QnDemoPrint(DEMO_WARNING, "%s[%d] parameter is null.\n", 
				__func__, __LINE__);
		return;
	}
	memcpy(_pUrl, &gAjMediaStreamConfig.rtmpConfig.appname, MAX_RTMP_APP_NAME_LEN);
}

//获取rtmp 推流的streamId
void GetStremId( char *_pStreamId )
{
    memcpy( _pStreamId, gAjMediaStreamConfig.rtmpConfig.streamid, MAX_RTMP_STREAMID_LEN );
}

//获取mqtt id
void GetServerAddr( char *_pServerAddr )
{
    memcpy( _pServerAddr, gAjMediaStreamConfig.rtmpConfig.server, MAX_IP_NAME_LEN );
}


