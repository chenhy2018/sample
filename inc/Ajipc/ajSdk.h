#ifndef __AJSDK_H__
#define __AJSDK_H__

#ifdef __cplusplus
extern "C" {
#endif

void AjInit();
void AjStart();
void AjStop();
int AjRelease();
void AjGetRtmpUrl(IN char *pUrl);
void GetStremId( char *_pStreamId );
void GetServerAddr( char *_pServerAddr );

#ifdef  __cplusplus
}
#endif

#endif
