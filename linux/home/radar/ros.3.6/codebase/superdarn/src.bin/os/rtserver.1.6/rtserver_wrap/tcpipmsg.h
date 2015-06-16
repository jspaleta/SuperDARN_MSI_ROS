/* tcpipmsg.h
   ==========
   Author: J.Spaleta & R.J.Barnes
*/

/*
 LICENSE AND DISCLAIMER
 
 Copyright (c) 2012 The Johns Hopkins University/Applied Physics Laboratory
 
 This file is part of the Radar Software Toolkit (RST).
 
 RST is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.
 
 RST is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with RST.  If not, see <http://www.gnu.org/licenses/>.
 
 
 
*/


#ifndef _TCPIPMSG_H
#define _TCPIPMSG_H

#define UNKNOWN_HOST 1
#define OPEN_FAIL 2
#define CONNECT_FAIL 3


struct TCPIPMsgHost {
  char host[256];
  int port;
  int sock;
};


int TCPIPMsgOpen(char *hostip, int port);
int  TCPIPMsgSend(int fd,void  *buf,size_t buflen);
int  TCPIPMsgRecv(int fd,void *buf,size_t buflen);

#endif
