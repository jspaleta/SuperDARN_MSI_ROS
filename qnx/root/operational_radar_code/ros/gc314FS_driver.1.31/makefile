OBJS= main.o include/_open_PLX9656.o include/_close_PLX9656.o \
	include/_config_PLLs.o include/_DMA_transfer.o\
	include/_reset_GC314FS.o include/_config_GC314FS.o \
	include/_config_GC4016.o include/_config_FPGA.o \
	include/_prog_conventions.o include/_create_DMA_buff.o \
	include/_read_GC4016.o include/_set_GC4016_vals.o \
	include/_change_freq.o include/_open_PLX9080.o \
	include/_DMA_collect.o include/_DMA_transfer_block.o \
	include/_DMA_transfer_sg.o include/_DMA_transfer_NODMA.o \
	include/_set_RDA.o
SRC= main.c include/_open_PLX9656.c include/_close_PLX9656.c \
	include/_config_PLLs.c include/_DMA_transfer.c\
	include/_reset_GC314FS.c include/_config_GC314FS.c \
	include/_config_GC4016.c include/_config_FPGA.c \
	include/_prog_conventions.c include/_create_DMA_buff.c \
	include/_read_GC4016.c include/_set_GC4016_vals.c \
	include/_change_freq.c include/_open_PLX9080.c \
	include/_DMA_collect.c include/_DMA_transfer_block.c \
	include/_DMA_transfer_sg.c include/_DMA_transfer_NODMA.c \
	include/_set_RDA.c
HDRS=	include/registers.h include/_prog_conventions.h \
	include/_regs_GC314FS.h include/_regs_GC4016.h \
	include/_regs_PLX9656.h include/_global.h \
	include/_regs_PLX9080.h ../include/gc314FS.h \
	include/_structures.h

hist: $(OBJS)
	cc -o _gc314FS $(OBJS) -lm -lc

main.o:			$(HDRS) main.c
_change_freq.o:		$(HDRS) _change_freq.c
_close_PLX9656.o: 	$(HDRS) _close_9656.c
_config_FPGA.o:		$(HDRS) _config_FPGA.c
_config_PLLs.o: 	$(HDRS) _config_PLLs.c _prog_conventions.c
_config_GC314FS.o: 	$(HDRS) _config_GC314FS.c
_config_GC4016.o: 	$(HDRS) _config_GC4016.c _prog_conventions.c
_create_DMA_buff.o: 	$(HDRS) _create_DMA_buff.c
_DMA_transfer.o:	$(HDRS) _DMA_transfer.c
_DMA_transfer_block.o:  $(HDRS) _DMA_transfer_block.c
_DMA_transfer_sg.o: 	$(HDRS) _DMA_transfer_sg.c
_DMA_transfer_NODMA.o:	$(HDRS) _DMA_transfer_NODMA.c
_DMA_collect.o:		$(HDRS) _DMA_collect.c
_open_PLX9080.o: 	$(HDRS) _open_9080.c 
_open_PLX9656.o: 	$(HDRS) _open_9656.c 
_prog_conventions.o: 	$(HDRS) _prog_conventions.c
_read_GC4016.o:		$(HDRS) _read_GC4016.c
_reset_GC314FS.o: 	$(HDRS) _reset_GC314FS.c
_set_GC4016_vals.o:	$(HDRS) _set_GC4016_vals.c
_set_RDA.o:		$(HDRS) _set_RDA.c
