OS:=$(shell uname -s)

CC=gcc
CFLAGS=-c -DPOSIX
common_libs=-lm  
linux_libs=-lfftw3 -lrt
qnx_libs=-lsocket -lc
INCLUDES=-I"../include/" -I"include/"  -I"../tsg/include/" -I"../fftw-3.2.2/api/" 
FFTW="../fftw-3.2.2/.libs/libfftw3.a"
SOURCES=main.c build.c setcard.c filter.c print_global.c\
         gc214.c gc4016.c io.c pci.c _create_DMA_buff.c\
         ../_tcpsocket.c ../utils.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=receiver_tcp_driver

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE):  $(OBJECTS)
ifeq ($(OS),Linux)
	$(CC) -o $@ $(OBJECTS)  $(common_libs) $(linux_libs) 
endif
ifeq ($(OS),QNX)
	$(CC) -o $@ $(OBJECTS) $(FFTW) $(common_libs) $(qnx_libs) 
endif

.c.o:
	$(CC) $(CFLAGS) $(LIBDIR) $< -o $@ $(INCLUDES) 

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)

