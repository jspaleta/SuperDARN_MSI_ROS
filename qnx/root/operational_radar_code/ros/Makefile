all: support drivers arbyserver  
support: iniparser fftw3
drivers: receiver dds timing diodriver gps 
iniparser:
	cd iniparser3.0b && $(MAKE) clean && $(MAKE) 
fftw3:
	cd fftw-3.2.2 && ./configure && $(MAKE) clean && $(MAKE)
arbyserver:
	cd server && ${MAKE} clean &&  ${MAKE} all
arbyclient:
	cd client && ${MAKE} clean  && ${MAKE} all
dds:
	cd ics660_drv && ${MAKE} clean  && ${MAKE}
	cd ddsserver_tcp_driver && ${MAKE} clean  && ${MAKE} all
receiver:
	cd gc314FS_driver.1.31 && ${MAKE} clean  && ${MAKE}
	cd gc316_tcp_driver && ${MAKE} clean  && ${MAKE} all
	cd gc214_tcp_driver && ${MAKE} clean  && ${MAKE} all
timing:
	cd timing_tcp_driver && ${MAKE} clean  && ${MAKE} all
diodriver:
	cd dio_tcp_driver && ${MAKE} clean  && ${MAKE} all
gps:
	cd gps_tcp_driver && ${MAKE} clean  && ${MAKE} all
	cd bc635_gps_tcp_driver && ${MAKE} clean  && ${MAKE} all

