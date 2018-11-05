//
// Created by chenh on 2018/6/7.
//

#ifndef QNSDK_COMMON_H
#define QNSDK_COMMON_H

#include <stdio.h>
#include <stdlib.h>


#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#define QN_TRUE 1
#define QN_FALSE 0

#define QN_FAIL (-1)
#define QN_SUCCESS (0)

#define DEMO_ERR (1)
#define DEMO_WARNING (2)
#define DEMO_NOTICE (3)
#define DEMO_INFO (4)
#define DEMO_DEBUG (5)

#define DEMO_PRINT DEMO_WARNING

void QnDemoPrint(IN int level, IN const char *fmt, ...);
#define BASIC() printf("[ %s %s() +%d ] ", __FILE__, __FUNCTION__, __LINE__ )
#define DBG_LOG(args...) BASIC();printf(args)


#define __compiler_offsetof(a,b) __builtin_offsetof(a,b)
#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE,MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) ({\
    const typeof(((type*)0)->member) *__mptr = (ptr);\
    (type *)((char *)__mptr - offsetof(type,member));})



#endif //QNSDK_COMMON_H
