#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <rpc/xdr.h>

#include <string.h>
#include <time.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
int main()
{
	int i;
	char buf[10];
	i=open("./server_tcp.c",O_RDONLY);
	printf("%d",i);
	read(i,buf,10);
	buf[9]=0;
	printf("%s",buf);
	/* code */
	return 0;
}