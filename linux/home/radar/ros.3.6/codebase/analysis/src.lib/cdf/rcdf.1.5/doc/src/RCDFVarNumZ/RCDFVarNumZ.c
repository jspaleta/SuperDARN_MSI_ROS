/* RCDFVarNumZ.c
   =============
   Author: R.J.Barnes
*/


#include <stdio.h>
#include <stdlib.h>
#include <cdf.h>
#include "rcdf.h"


int main(int argc,char *argv[]) {

  CDFid id;
  CDFstatus status;
  int num;

  fprintf(stdout,"RCDFVarNumZ\n");
  if (argc==1) {
    fprintf(stderr,"No filename.\n");
    exit(-1);
  } 

  status=CDFopen(argv[1],&id);
  if (status !=CDF_OK) {
     fprintf(stderr,"Could not open cdf file.\n");
     exit(-1);
  }

  num=RCDFVarNumZ(id);

  CDFclose(id);
  
  fprintf(stdout,"Z var num. :%d\n",num);

  return 0;
}
