#ifndef __QNBEAT_H__
#define __QNBEAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "OnMsg.h"
#include "QnMqttConn.h"


void BeatInit();
void BeatStart();
void BeatStop();

#ifdef __cplusplus
}
#endif

#endif
