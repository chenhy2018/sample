#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "QnMqttConn.h"
#include "QnBeat.h"

/*
 * function: sigalarm_handler
 * description: signal ALARM deal handler
 * */
void sigalarm_handler(int signo)
{
    struct MqttConn *conn = NewMqttConn();
    if (!conn) {
        return;
    }
    struct OnMsg *msg = NewOnMsg("001", "MQHB", 1);
    if (!msg) {
        goto SIGALARM_HANDLER_FREECONN;
    }

    msg->base.ops->CreateMsg(&msg->base.elt);
    conn->base.ops->ConnWrite(&conn->base, msg->base.elt.data, sizeof(msg->base.elt.data));
    DeleteOnMsg(msg);
    
SIGALARM_HANDLER_FREECONN:
    DeleteMqttConn(conn);
}

/*
 * function: beat_timer_init
 * description: init real timer
 * */
static void beat_timer_init()
{
    signal(SIGALRM, sigalarm_handler);
}

/*
 * function: beat_set_timer
 * description: set real timer interval
 * */
static void beat_set_timer(struct timeval interval)
{
    struct itimerval timer;

    timer.it_interval.tv_sec = interval.tv_sec;
    timer.it_interval.tv_usec = interval.tv_usec;
    timer.it_value.tv_sec = interval.tv_sec;
    timer.it_value.tv_usec = interval.tv_usec;
    // setitimer(TIMER_VIRTUAL, &timer, NULL);
    setitimer(ITIMER_REAL, &timer, NULL);
}

/*
 * function: beat_clean_timer
 * description: clean real timer
 * */
static void beat_clean_timer()
{
    struct itimerval timer = {};

    // gettitimer(ITIMER_REAL, &timer);
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
}

void BeatInit()
{
    beat_timer_init();
}

void BeatStart()
{
    struct timeval interval = {
        .tv_sec = 1,
        .tv_usec = 0,
    };

    beat_set_timer(interval);
}

void BeatStop()
{
    beat_clean_timer();
}