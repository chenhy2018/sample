#ifndef __QNAVCHANNEL_H__
#define __QNAVCHANNEL_H__

#ifdef __cplusplus
extern "C" {
#endif


struct AVChannel 
{
    struct AVChannelOps ops;
};

struct AVChannelOps
{
    int (*ChanOpen)(struct AVChannel *chan);
    int (*ChanClose)(struct AVChannel *chan);
    int (*ChanSend)(struct AVChannel *chan);
};


#ifdef __cplusplus
}
#endif
#endif