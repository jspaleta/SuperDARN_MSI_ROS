ICS660_OBJS = ics660_drv_init.o get_pci_base_addr.o dc60m_init.o init_gc4116.o one_shot_a.o one_shot_b.o set_dc60m_resets.o select_chip.o channel_flush.o assign_register_addresses.o set_parameter.o load_frequency.o load_phase.o load_filter_taps.o ideal_filter_coef.o set_dc_ready.o release_dc60m_resets.o print_registers.o pdebug.o

.c.o:
	cc -c -I"./" -I"../ddsserver_tcp_driver/include" -o $@ $<

_ics660_drv:	_ics660-drv.o $(ICS660_OBJS)
	cc -o _ics660-drv _ics660-drv.o $(ICS660_OBJS) -lm

