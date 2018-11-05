#include "QnMsg.h"


static int msg_send(struct Msg *msg)
{
    if (!msg || !msg->data || !msg->conn ) {
        return QN_FAIL;
    }

    msg->conn->ops->ConnWrite(msg->conn, msg->data, msg->len);
    return QN_SUCCESS;
}


struct Msg *NewMsg(struct Conn *conn)
{
    struct Msg *msg = (struct msg *)malloc(sizeof(struct Msg));
    if (msg) {
        msg->SendMsg = msg_send;
        msg->conn = conn;
    }
    return msg;
}

void DeleteMsg(struct Msg *msg)
{
    if (msg) {
        free(msg);
    }
}

void TestMsg(struct Conn *conn)
{
    struct Msg *msg = NewMsg(conn);
    if (msg){
        msg->SendMsg(msg);
    }
}