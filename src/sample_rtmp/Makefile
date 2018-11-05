#include $(RULESFILE)

#DEMO_LIBA = $(PROJECT_DIR)/lib/libua.a $(PROJECT_DIR)/lib/librtmp_sdk.a $(PROJECT_DIR)/lib/librtmp.a $(PROJECT_DIR)/lib/libfdk-aac.a  $(PROJECT_DIR)/lib/external/libdevsdk.a $(PROJECT_DIR)/lib/external/libtools.a 

#DEMO_LIBS =-lm  -lstdc++  -lpthread
#INCLUDE_PATH = -I$(PROJECT_DIR)/inc -I$(PROJECT_DIR)/inc/Ajipc 
BUILD_MODE = release
DEMO_LIBA :=
DEMO_LIBS :=
INCLUDE_PATH = -I.
#CC = gcc
#STRIP = strip
CROSS_TOOLDIR = /home/samba/toolChain/arm-linux-gnueabihf-4.8.3-201404/bin
CROSS_COMPILE = arm-linux-gnueabihf-
CC = $(CROSS_TOOLDIR)/$(CROSS_COMPILE)gcc
STRIP = $(CROSS_TOOLDIR)/$(CROSS_COMPILE)strip
#PRJ_DIR =/opt2/AJ_Prj/tmp_git/sample/src/sample_rtmp
PRJ_DIR =/home/samba/iWork/AJ_Prj/tmp_git/sample/src/sample_rtmp
$(warning "+++++++++"$(PRJ_DIR))
DEMO_LIBS = -lpthread -lm
DEMO_LIBS += -L. -lmqtt
DEMO_LIBA = $(PRJ_DIR)/libdevsdk.a $(PRJ_DIR)/libtools.a $(PRJ_DIR)/librtmp_sdk.a $(PRJ_DIR)/librtmp.a $(PRJ_DIR)/libfdk-aac.a
OBJS= main.o OnMsg.o QnMqttConn.o QnBeat.o AjFrameHandlerSDK.o QnRtmpFrameHandler.o QnCommon.o QnMqttConf.o QnRtmpConf.o #QnMsg.o QnMqttConn.o QnConn.o QnMqttConf.o QnSysConf.o QnRtmpConf.o QnConf.o QnCommon.o


ifeq ($(BUILD_MODE), debug)
.c.o:
	$(CC) $(INCLUDE_PATH) -g -c $<
else
.c.o:
	$(CC) $(INCLUDE_PATH) -c $<
endif


demo_rtmp:$(OBJS)
	#$(CC) -o $@ $^ $(LDFLAGS)
	$(CC)  -o $@ $^ $(DEMO_LIBA) $(DEMO_LIBS)
	$(STRIP) $@
	rm *.o

clean:
	rm  demo_rtmp *.o

