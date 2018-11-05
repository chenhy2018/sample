#ifndef __QNMSG_H__
#define __QNMSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "QnCommon.h"
#include "QnConn.h"

#define MSG_UA_LEN 8
#define MSG_MODULE_LEN 8
#define MSG_LEN 64

struct MsgOperations 
{
    void (*CreateMsg)(struct Msg *msg);
};

struct MsgElement
{
    char ua[MSG_UA_LEN];
    char module[MSG_MODULE_LEN];
    int val;
    char data[MSG_LEN];
};

struct Msg
{
    // struct Conn *conn;   //通信连接
    struct MsgOperations *ops;  //接口操作
    struct MsgElement elt; //数据
    // void (*SendMsg)(); //和conn 进行数据发送,用于透明化conn   
};

struct Msg *NewMsg();
void DeleteMsg(struct Msg *msg); 
void TestMsg(struct Conn *conn);

#ifdef __cplusplus
}
#endif
#endif