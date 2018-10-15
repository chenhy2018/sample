#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "QnCommon.h"
#include "QnRtmp.h"
#include "ajSdk.h"
#include "QnMqtt.h"
#include "main.h"

int main()
{
    /* initial signal set */
    //信号处理初始化
    int sig;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGPIPE);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
	
	//InitSigHandler();
    //初始化mqtt log互斥锁
    InitMqttMutex();
    //Aj相关初始化
	AjInit();
    //Qn mqtt初始化
	QnMqttInit();
    //Qn rtmp初始化 
	RtmpInit();

    //start
	QnMqttStart();
	AjStart();
	
	while(1) {
        sigwait(&set, &sig);
        switch (sig) {
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
			RtmpRelease();
			AjStop();
			AjRelease();
            break;
        default:
            QnDemoPrint(DEMO_NOTICE, "unmasked signal %d catch.", sig);
            continue;
        }
	}
		
    DestroyMqttMutex();
    

	return 0;
}
