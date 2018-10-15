include $(RULESFILE)

$(warning =====$(CROSS_TOOLDIR))
DEMO_LIBA = $(PROJECT_DIR)/lib/libua.a $(PROJECT_DIR)/lib/librtmp_sdk.a $(PROJECT_DIR)/lib/librtmp.a $(PROJECT_DIR)/lib/libfdk-aac.a  $(PROJECT_DIR)/lib/external/libdevsdk.a $(PROJECT_DIR)/lib/external/libtools.a 

DEMO_LIBS =-lm  -lstdc++  -lpthread
INCLUDE_PATH = -I$(PROJECT_DIR)/inc -I$(PROJECT_DIR)/inc/Ajipc 

CC = $(CROSS_TOOLDIR)/$(CROSS_COMPILE)gcc
OBJS= main.o QnRtmp.o AjSdk.o QnCommon.o QnMqtt.o


ifeq ($(BUILD_MODE), debug)
.c.o:
	$(CC) $(INCLUDE_PATH) -g -c $<
else
.c.o:
	$(CC) $(INCLUDE_PATH) -c $<
endif


demo_rtmp:$(OBJS)
	#$(CC) -o $@ $^ $(LDFLAGS)
	$(CC) -o $@ $^ $(DEMO_LIBA) $(DEMO_LIBS)
	rm *.o

clean:
	rm  demo_rtmp

