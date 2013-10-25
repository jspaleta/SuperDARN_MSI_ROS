#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>

extern int verbose;
int tcpsocket(int port){

	time_t	tod;
	
	int	sock,length;
	struct	sockaddr_in	servertx;
	int	msgsock;
	char	buf[1024];
	int	i;
	struct	timespec start,stop;
	float	ftime;

	int 	temp, option, optionlen;


	/* CREATE SOCKET */
	sock=socket(AF_INET, SOCK_STREAM, 0);
	if( (sock<0) ){
		perror("opening stream socket\n");
		exit(1);
	}

	/* NAME SOCKET USING WILDCARDS */
	servertx.sin_family=AF_INET;
	servertx.sin_addr.s_addr=INADDR_ANY;
	servertx.sin_port=htons(port);
	if( bind(sock, (struct sockaddr *)&servertx, sizeof(servertx)) ){
		perror("binding tx stream socket");
		exit(1);
	}
	/* FIND ASSIGNED PORT NUMBER AND PRINT IT */
	length=sizeof(servertx);
	if( getsockname(sock, (struct sockaddr *)&servertx, &length) ){
		perror("getting sock name");
		exit(1);
	}
	if (verbose > 0)  printf("GPS Socket using TCP port #%d\n", ntohs(servertx.sin_port));

	optionlen=4;
	option=TCP_NODELAY;
	temp=setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&option,optionlen);
	temp=getsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&option,&optionlen);
	if (verbose> 1) printf("temp=%d  optionlen=%d  option=%d\n",temp,optionlen,option);
	optionlen=4;
	option=32768;
	//temp=setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&option,optionlen);
	temp=setsockopt(sock,SOL_SOCKET,SO_SNDBUF,&option,optionlen);
	temp=getsockopt(sock,SOL_SOCKET,SO_SNDBUF,&option,&optionlen);
	if (verbose> 1) printf("temp=%d  optionlen=%d  option=%d\n",temp,optionlen,option);
	optionlen=4;
	option=32768;
	//temp=setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&option,optionlen);
	temp=setsockopt(sock,SOL_SOCKET,SO_RCVBUF,&option,optionlen);
	temp=getsockopt(sock,SOL_SOCKET,SO_RCVBUF,&option,&optionlen);
	if (verbose> 1) printf("temp=%d  optionlen=%d  option=%d\n",temp,optionlen,option);

	/* return the socket */
	temp=1;
	//ioctl(sock,FIONBIO,&temp);
	return sock;
}


