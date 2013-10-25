OS:=$(shell uname -s)

CC=gcc
CFLAGS=-c
LFLAGS  = -L"../iniparser3.0b"
common_libs=-lm -liniparser
linux_libs=
qnx_libs=-lsocket
INCLUDES=-I"../include/" -I"include/" -I"../tsg/include" -I"../iniparser3.0b/src/"  
SOURCES=main.c  _dma.c _open_PLX9080.c _create_DMA_buff.c _decodestate.c _open_ini_file.c ../_tcpsocket.c ../utils.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=timing_tcp_driver

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
ifeq ($(OS),Linux)
	$(CC) -o $@ $(OBJECTS) $(LFLAGS) $(common_libs) $(linux_libs) 
endif
ifeq ($(OS),QNX)
	$(CC) -o $@ $(OBJECTS) $(LFLAGS) $(common_libs) $(qnx_libs) 
endif

.c.o:
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(INCLUDES)

clean:
	rm -rf $(OBJECTS) timing_tcp_driver

