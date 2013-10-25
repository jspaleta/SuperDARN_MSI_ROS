OS:=$(shell uname -s)

CC=gcc
CFLAGS=-c
common_libs=-lm 
linux_libs=-lrt -lpthread
qnx_libs=-lsocket
INCLUDES=-I"include/" -I"../include/" -I"../tsg/include/" 
SOURCES=main.c ../utils.c _open_PCI.c _close_PCI.c bc635_functions.c \
	_prog_conventions.c _get_event_time.c _tcpsocket.c\
	_get_software_time.c _refresh_state.c uio_helper.c printbits.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=gps_tcp_driver

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
	rm -rf $(OBJECTS) gps_tcp_driver

