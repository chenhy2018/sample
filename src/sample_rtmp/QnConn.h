#ifndef __QNCONN_H_
#define __QNCONN_H_

#ifdef __cplusplus
extern "C" {
#endif


struct Conn
{
    struct ConnOps *ops;
};

struct ConnOps
{
    void (*ConnOpen)(struct Conn *this);
    void (*ConnWrite)(struct Conn *this, const char *msg, int len);
    void (*ConnRead)(struct Conn *this);
    void (*ConnClose)(struct Conn *this);
    void (*ConnInit)();
    void (*ConnExit)();
};


#ifdef __cplusplus
}
#endif
#endif
