#include <stdlib.h>
#include <stdio.h>
#include "QnCommon.h"
#include "QnConf.h"
#include "QnSysConf.h"


static int sys_getconf(struct Conf *this)
{
    if (!this) {
        return QN_FAIL;
    }
    struct SysConf *self = container_of(this, struct SysConf, base);
    memcpy(&self->ua[0], "hello", sizeof("hello"));
    
    return QN_SUCCESS; 
}

static const struct ConfOps gSysConfOps = {
    .GetConf =  sys_getconf,
};

struct SysConf *NewSysConf(struct Conf *this)
{
    struct SysConf *self = (struct SysConf *)malloc(sizeof(struct SysConf));
    if (self) {
        memset(self, 0, sizeof(struct SysConf));
        self->base.ops = &gSysConfOps;
        self->base.next = this;
    }
    return self;
}

void DeleteSysConf(struct SysConf *self)
{
    if (self) {
        free(self);
    }
}