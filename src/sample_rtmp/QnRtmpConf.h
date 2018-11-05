#ifndef __QNRTMPCONF_H__
#define __QNRTMPCONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "QnCommon.h"
#include "QnConf.h"

#define RTMP_ELEMENT_LEN 256

struct RtmpDefaultcfg
{
    char url[RTMP_ELEMENT_LEN];
};

struct RtmpConf 
{
    char *url;
    struct Conf base;
};

struct RtmpConf *NewRtmpConf(struct Conf *this);
void DeleteRtmpConf(struct RtmpConf *self);
void set_rtmpconfig(struct RtmpDefaultcfg cfg);

#ifdef __cplusplus
}
#endif
#endif