#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int writetcp(int sock, char *buff, int size){

	int bytes_written, to_write,temp;

	bytes_written=0;
	while( bytes_written < size ){
		to_write=size-bytes_written;
		buff+=bytes_written;
		bytes_written+=send(sock,buff,to_write,0);
		//temp=write(sock,buff,to_write);
		//printf("temp=%d   to_write=%d   bytes_written=%d   size=%d\n",temp,to_write,bytes_written,size);
		//bytes_written+=temp;
	}
	return bytes_written;
}

int readtcp(int sock, char *buff, int size){

	int bytes_read, to_read;

	bytes_read=0;
	while( bytes_read < size ){
		to_read=size-bytes_read;
		buff+=bytes_read;
		bytes_read+=recv(sock,buff,to_read,MSG_WAITALL);
		//bytes_read+=read(sock,buff,to_read);
	}
	return bytes_read;
}

