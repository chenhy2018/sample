export PROJECT_DIR=$(shell pwd)
export RULESFILE=$(PROJECT_DIR)/rules.make

include $(RULESFILE)

#export $(PROJECT_DIR)
#export $(RULESFILE)

all:   
	@for dir in $(MODULELIST);do $(MAKE) -C ./src/$$dir;done

clean:
	make clean -C ./src/sample_rtmp
menuconfig:
	script/mconf script/sample_config
