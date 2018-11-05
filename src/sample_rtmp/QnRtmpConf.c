#include "QnRtmpConf.h"

static struct RtmpDefaultcfg gRtmpDefaultcfg = {
    .url = "http://qiniu.com",
};

void set_rtmpconfig(struct RtmpDefaultcfg cfg)
{
    memcpy(&gRtmpDefaultcfg.url[0], &cfg.url[0], RTMP_ELEMENT_LEN);
    QnDemoPrint(DEMO_INFO, "%s:%d>> rtmp url:%s\n", __func__, __LINE__, &gRtmpDefaultcfg.url[0]);
}

static char * rtmp_geturl()
{
    return &gRtmpDefaultcfg.url[0];
}

static int rtmp_getconf(struct Conf *this)
{
    if (!this) {
        return QN_FAIL;
    }
   struct RtmpConf *self = container_of(this, struct RtmpConf, base);
   self->url = rtmp_geturl();
   if (this->next && this->next->ops && this->next->ops->GetConf) {
       this->next->ops->GetConf(this->next);
   }
   
   return QN_SUCCESS;
}

static const struct ConfOps gRtmpConfOps = {
    .GetConf = rtmp_getconf,
};

struct RtmpConf *NewRtmpConf(struct Conf *this)
{
    struct RtmpConf *self = (struct RtmpConf *)malloc(sizeof(struct RtmpConf));
    if (self) {
        memset(self, 0, sizeof(struct RtmpConf));
        self->base.ops = &gRtmpConfOps;
        self->base.next = this;
    }
    return self;
}

void DeleteRtmpConf(struct RtmpConf *self)
{
    if (self) {
        free(self);
    }
}