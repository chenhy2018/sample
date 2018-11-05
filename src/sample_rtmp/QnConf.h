#ifndef __QNCONF_H__
#define __QNCONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>


struct Conf
{
    struct ConfOps *ops;
    struct Conf *next;
};

struct ConfOps
{
    void (*Init)(struct Conf *self);
    int (*GetConf)(struct Conf *self);
    int (*SetConf)(struct Conf *self); 
};

#ifdef __cplusplus
}
#endif

#endif