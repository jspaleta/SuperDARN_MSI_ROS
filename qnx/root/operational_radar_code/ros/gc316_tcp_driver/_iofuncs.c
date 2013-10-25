
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#ifdef __QNX__
  #include <sys/iofunc.h>
  #include <sys/dispatch.h>
  #include <devctl.h>
  #include <sched.h>
  #include <process.h>
  #include <sys/iomsg.h>
  #include <sys/uio.h>
  #include <sys/resmgr.h>
  #include <sys/neutrino.h>
  #include <hw/inout.h>
  #include <hw/pci.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
#endif

#include "global_server_variables.h"
#include "utils.h"
#include "gc314FS_defines.h"
#include "gc314FS.h"
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"


extern int verbose;
typedef union _devctl_msg {
	int tx;
	int rx;
} data_t;

unsigned int	 virtual_addr[MAX_CARDS], physical_addr[MAX_CARDS];
int		 sample_count=0,overflow=0, loops,samples_to_collect,remaining_samples,DMA_switch=0;
unsigned int	 BASE0[MAX_CARDS], BASE1[MAX_CARDS], *BASE1_phys;
unsigned char	 *BASE0_dio, *io_BASE1_dio, *BASE1_phys_dio;

//extern unsigned int virtual_addr[MAX_CARDS], physical_addr[MAX_CARDS];
//extern int sample_count,overflow, loops,samples_to_collect;

int
io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
  int nleft;
  int nbytes;
  int nparts;
  int status;
  uint32_t chip;
  //char *lbuff="This is just a test!";
  char lbuff[2048];
  char tempbuff[2048];
  uint32_t cval;
  int return_len;
  int tempint;

  if ((status = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK)
    return(status);

  if ((msg->i.xtype & _IO_XTYPE_NONE) != _IO_XTYPE_NONE)
    return (ENOSYS);

  //set up 'string' of data to return
  fprintf(stderr,"io_read - cval %d\n",cval);
  //tempint=*((uint08*)(gc314base+GC314FS_GC1offset+GC4016_GLOBAL_RESET));
/*
  sprintf(tempbuff,"GLOBAL_RESET Chip 1 = 0x%02x\n", *((uint08*)(gc314base+GC314FS_GC1offset+GC4016_GLOBAL_RESET)) );
  strcat(lbuff,tempbuff);
  sprintf(tempbuff,"GLOBAL_RESET Chip 2 = 0x%02x\n", *((uint08*)(gc314base+GC314FS_GC2offset+GC4016_GLOBAL_RESET)) );
  strcat(lbuff,tempbuff);
  sprintf(tempbuff,"GLOBAL_RESET Chip 3 = 0x%02x\n", *((uint08*)(gc314base+GC314FS_GC3offset+GC4016_GLOBAL_RESET)) );
  strcat(lbuff,tempbuff);
  lbuff[80]=0;
*/

  //sprintf(lbuff,"%s","just testing what I can");
  printf("%s\n", lbuff);
  return_len=strlen(lbuff)+1;
  printf("return length is %d\n", return_len);
  
  //nleft = ocb->attr->nbytes - ocb->offset;
  nleft = return_len - ocb->offset;
  nbytes = min(msg->i.nbytes, nleft);

  if (nbytes > 0){
    /* set up the return data IOV */
    SETIOV( ctp->iov, lbuff + ocb->offset, nbytes );

    /* set up the number of bytes (returned by client's read()) */
    _IO_SET_READ_NBYTES(ctp, nbytes);

    /*
     * advance the offset by the number of bytes 
     * returned to the client.
     */

    ocb->offset += nbytes;

    nparts = 1;
  } else {
    /*
     * they've asked for zero bytres or they've already 
     * read everything
     */
    _IO_SET_READ_NBYTES(ctp,0);

    nparts = 0;
  }

  /* mark the access time as invalid (we just accessed it) */
  if (msg->i.nbytes > 0)
    ocb->attr->flags |= IOFUNC_ATTR_ATIME;
 
  return (_RESMGR_NPARTS(nparts));
}

int
io_write( resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
  
  /*
   * On all reads, calculate how many
   * bytes we can return to the client
   * based upon the number of bytes available (nleft)
   * and the client's buffer size
   */

  int status;
  char *buf;

  if((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK )
    return(status);

  if((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
    return(ENOSYS);

  _IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);

  buf = (char *)malloc(msg->i.nbytes + 1);
  if( buf == NULL)
    return(ENOMEM);

  /*
   * reread the data from the sender's message buffer.
   * We're not assuming that all of the data fit into the 
   * resource manager library's receive buffer.
   */
  if( (status = resmgr_msgread(ctp, buf, msg->i.nbytes, sizeof(msg->i))) == -1){ 
    return(ENOSYS);
  }
  fprintf(stderr,"_ics660-drvr:  bytes attemted: %d  bytes received %d\n",msg->i.nbytes,status);
  buf[msg->i.nbytes] = '\0'; //just in case text is not NULL terminated 
//  memcpy((int *)ics660->mem1,buf,(size_t)msg->i.nbytes);
  free(buf);

  if(msg->i.nbytes > 0)
    ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

  return(_RESMGR_NPARTS(0));
}


