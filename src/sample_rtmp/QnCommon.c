//
// Created by chenh on 2018/6/19.
//
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "QnCommon.h"

#define ARGSBUF_LEN 256

void DemoPrintTime(void)
{
    struct tm *tblock = NULL;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    tblock = localtime(&tv.tv_sec);
    printf("%d/%d/%d %d:%d:%d:%d",
           1900+tblock->tm_year, 1+tblock->tm_mon,
           tblock->tm_mday, tblock->tm_hour,
           tblock->tm_min, tblock->tm_sec, tv.tv_usec);
}

void QnDemoPrint(IN int level, IN const char *fmt, ...)
{
    if (level <= DEMO_PRINT) {
        DemoPrintTime();

        va_list args;
        char argsBuf[ARGSBUF_LEN] = {0};
        if (argsBuf == NULL) {
            return;
        }
        va_start(args, fmt);
        vsprintf(argsBuf, fmt, args);
        printf(">>%s\n", argsBuf);
        va_end(args);
		
    }
}

void signal_wait_loop(sigset_t *set, int *sig)
{
    while (1) {
        sigwait(set, sig);
        switch (*sig) {
        case SIGALRM:
            QnDemoPrint(DEMO_NOTICE, "SIGALRM catch.");
            continue;
        case SIGPIPE:
            QnDemoPrint(DEMO_NOTICE, "SIGPIPE catch.");
            continue;
        case SIGUSR1:
            QnDemoPrint(DEMO_NOTICE, "SIGUSR1 catch.");
            continue;
        case SIGUSR2:
            QnDemoPrint(DEMO_NOTICE, "SIGUSR2 catch.");
            continue;
        case SIGINT:
            QnDemoPrint(DEMO_NOTICE, "SIGINT catch, exiting...");
            break;
        default:
            QnDemoPrint(DEMO_NOTICE, "unmasked signal %d catch.", *sig);
            continue;
        }
    }
}
