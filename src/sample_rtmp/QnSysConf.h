#ifndef __QNSYSCONF_H__
#define __QNSYSCONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "QnConf.h"

#define SYS_ELEMENT_LEN 128

struct SysConf
{
    char ua[SYS_ELEMENT_LEN];
    struct Conf base;
};

struct SysConf *NewSysConf(struct Conf *this);
void DeleteSysConf(struct SysConf *self);

#ifdef __cplusplus
}
#endif

#endif