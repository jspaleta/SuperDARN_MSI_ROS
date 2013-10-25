OS:=$(shell uname -s)

CC=gcc
CFLAGS=-c
common_libs=-lm 
linux_libs=
qnx_libs=-lsocket
INCLUDES=-I"../include/" -I"include/" -I"../tsg/include" -I"../server/include"  
SOURCES=main.c  _dma.c _open_PLX9080.c _create_DMA_buff.c _decodestate.c ../_tcpsocket.c ../utils.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=timing_tcp_driver

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
ifeq ($(OS),Linux)
	$(CC) -o $@ $(OBJECTS) $(common_libs) $(linux_libs) 
endif
ifeq ($(OS),QNX)
	$(CC) -o $@ $(OBJECTS) $(common_libs) $(qnx_libs) 
endif

.c.o:
	$(CC) $(CFLAGS) $< -o $@ $(INCLUDES)

clean:
	rm -rf $(OBJECTS) timing_tcp_driver

