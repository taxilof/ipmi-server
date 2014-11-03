

# config
CC      = /usr/bin/gcc
LDFLAGS =  -lcrypto
CFLAGS_DEBUG 	= -std=c99	-Wall -g -DDEBUG -D_REENTRANT
CFLAGS_NORMAL 	= -std=c99 -O3 
CFLAGS 			= $(CFLAGS_NORMAL)
# objects
OBJ = ipmi-msg.o ipmi-sdr.o ipmi-sensor.o ipmi-session.o udp-server.o asf.o rmcp.o
%.o: %.c
	$(CC) $(CFLAGS) -c $<




	
ipmi-server : $(OBJ)
	$(CC) $(CFLAGS)   -o $@ $(OBJ) $(LDFLAGS)

ipmi-server-rpi:  		CFLAGS += -DTARGET_RPI
ipmi-server-rpi : 		ipmi-server

ipmi-server-odroid:  	CFLAGS += -DTARGET_ODROID
ipmi-server-odroid: 	ipmi-server

		


# clean before make special targets
rpi: clean_o ipmi-server-rpi
odroid: clean_o ipmi-server-odroid
normal: clean_o ipmi-server
all: normal-debug

# debug stuff
rpi-debug: 		CFLAGS = $(CFLAGS_DEBUG)
rpi-debug: 		rpi
odroid-debug: 	CFLAGS = $(CFLAGS_DEBUG)
odroid-debug: 	odroid
normal-debug:	CFLAGS = $(CFLAGS_DEBUG)
normal-debug: 	normal

  
      
# default stuff
.PHONY: clean clean_o clean_bin 
clean: clean_o clean_bin
	
clean_o:
	-rm *.o 
clean_bin:
	-rm ipmi-server

