OS:=$(shell uname -s)

CC=gcc
CFLAGS=-c -DPOSIX
common_libs=-lm 
linux_libs=-lfftw3 -lrt -lpthread
qnx_libs=-lsocket -lc
INCLUDES=-I"../include/" -I"include/" -I"../tsg/include" -I"../server/include" -I"../fftw-3.2.2/api/" 
FFTW="../fftw-3.2.2/.libs/libfftw3.a"
SOURCES=main.c  _open_PLX9656.c  _create_DMA_buff.c gc314GetBufferAddress.c \
        gc314ChannelOn.c gc314ChannelAbort.c gc314LoadGC4016s.c \
        gc314SetExternalTrigger.c gc314SetFilters.c gc314SetFrequency.c \
        gc314SetGlobalReset.c gc314SetOutputRate.c gc314SetRDA.c \
        gc314SetSamples.c gc314SetSync1.c gc314SetSync2.c gc314SetSyncMask.c \
        gc314StartCollection.c gc314UpdateChannel.c gc314WaitForData.c \
         ../_tcpsocket.c ../utils.c ../beam_phase.c
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

