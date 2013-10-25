#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#ifdef _QNX_
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#endif
#include <sys/mman.h>

int _close_PLX9656(int pci_handle, int BASE0, int BASE1, int mmap_io_ptr){

	int temp;
#ifdef __QNX__
	pci_detach(pci_handle);
	temp=munmap_device_memory((unsigned int *)BASE0, 256);
	if (temp==-1){
		perror("munmap_device_memory failed");
		return -1;
	}
	temp=munmap_device_memory((unsigned int *)BASE1, 524288);
	if (temp==-1){
		perror("munmap_device_memory failed");
		return -1;
	}
	temp=munmap_device_io(mmap_io_ptr,256);
	if (temp==-1){
		perror("munmap_device_io failed");
		return -1;
	}
#endif
	return 1;
}
