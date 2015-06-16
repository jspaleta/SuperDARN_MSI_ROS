%module rtserver
#include <stdio.h>
%include "typemaps.i"


%typemap(in) (int argc, char *argv[]) {
      int i = 0;
      if (!PyList_Check($input)) {
        PyErr_SetString(PyExc_ValueError, "Expecting a list");
        return NULL;
      }
      $1 = PyList_Size($input);
      $2 = (char **) malloc(($1+1)*sizeof(char *));
      for (i = 0; i < $1; i++) {
        PyObject *s = PyList_GetItem($input,i);
        if (!PyString_Check(s)) {
            PyErr_SetString(PyExc_ValueError, "List items must be strings");
            return NULL;
        }
        $2[i] = PyString_AsString(s);
      }
      $2[i] = 0;
}

%typemap(freearg) (int argc, char *argv[]) {
       free($2); 
}

%typemap(out) int16 {
    	$result = PyInt_FromLong($1);
}

%typemap(out) int32 {
    	$result = PyInt_FromLong($1);
}


%{
#include <zlib.h>

#include "rtypes.h"
#include "option.h"
#include "tcpipmsg.h"

#include "dmap.h"
#include "rprm.h"
#define complex Complex

#include "fitdata.h"
#include "fitblk.h"
#include "radar.h"
#include "rmsg.h"
#include "rmsgrcv.h"
#include "errlog.h"

#include "fitpacket.h"
#include "fork.h"

#include "version.h"
#include "rtserver.h"
%}


%include "rtypes.h"
%include "option.h"
%include "tcpipmsg.h"

%include "dmap.h"
%include "rprm.h"
#define complex Complex

%include "fitdata.h"
%include "fitblk.h"

%include "radar.h"
%include "rmsg.h"
%include "rmsgrcv.h"
%include "errlog.h"

%include "fitpacket.h"
%include "fork.h"

%include "version.h"
%include "rtserver.h"


