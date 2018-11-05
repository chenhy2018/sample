#include <string.h>
#include "QnCommon.h"
#include "QnMsg.h"
#include "OnMsg.h"

static int onmsg_CreateMsg(struct MsgElement *elt)
{
    if (!elt) {
        QnDemoPrint(DEMO_ERR, "%s:%d>> param is null.\n", __func__, __LINE__);
        return QN_FAIL;
    }
    sprintf(elt->data, "%s %s %d", elt->ua, elt->module, elt->val);
    QnDemoPrint(DEMO_ERR, "%s:%d>> Beat msg:%s\n", __func__, __LINE__, elt->data);
    return QN_SUCCESS;
}

static const struct MsgOperations gOnMsgOps = {
    .CreateMsg = onmsg_CreateMsg,
};

struct OnMsg *NewOnMsg(char *ua, char *module, int val)
{
    struct OnMsg *msg = (struct OnMsg *)malloc(sizeof(struct OnMsg));
    if (msg) {
        memset(msg, 0, sizeof(struct OnMsg));
        msg->base.ops = &gOnMsgOps;
        memcpy(msg->base.elt.ua, ua, MSG_UA_LEN);
        memcpy(msg->base.elt.module, module, MSG_MODULE_LEN);
        msg->base.elt.val = val;
    }
}

void DeleteOnMsg(struct OnMsg *msg)
{
    // if (msg->onmsg_release) {
    //     msg->onmsg_release(msg);
    // }
    free(msg);
}