#ifndef __QNFRAMEHANDLERSDK_H__
#define __QNFRAMEHANDLERSDK_H__

#ifdef __cplusplus
extern "C" {
#endif

struct FrameHandlerSdkOperation 
{
    void (*Init)(struct FrameHandlerSdk *sdk);
    void (*Release)(struct FrameHandlerSdk *sdk);
    void (*Register)(struct FrameHandlerSdk *sdk, int module, void *callback);
    void (*UnRegister)(struct FrameHandlerSdk *sdk);
    int (*Start)(struct FrameHandlerSdk *sdk);
    int (*Stop)(struct FrameHandlerSdk *sdk);
};

struct FrameHandlerSdk 
{
    void *cfg;
    struct FrameHandlerSdkOperation *ops;
    struct FrameHandler *framehandler;
};


#ifdef __cplusplus
}
#endif

#endif