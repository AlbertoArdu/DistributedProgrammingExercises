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
#include <sys/time.h>

#include "errlib.h"


#define MAX_BUF 50
#define TIMEOUT 15

int main(int argc, char const *argv[]){
	uint16_t porta;
	int sock;
	socklen_t len;
	struct sockaddr_in indirizzo, mittente;
	char buf_ric[MAX_BUF];

	//Strutture dati per controllare il timeout
	struct timeval tval;
	fd_set cset;
	int val,n=5,t=TIMEOUT;
	FD_ZERO(&cset);
	tval.tv_sec=t;
	tval.tv_usec =0;
	if(argc<4){
		fprintf(stderr, "Formato: %s ip porta stringa\n", argv[0]);
		exit(1);
	}

	if(!inet_pton(AF_INET,argv[1], &(indirizzo.sin_addr)))
        err_quit("indirizzo non valido");

    porta=htons(atoi(argv[2]));
    indirizzo.sin_family = PF_INET;
    indirizzo.sin_port = porta;


    sock = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP);

    do{
    	printf("Sto inviando la stringa al server...\n");
    	if(sendto(sock,argv[3],strlen(argv[3]),0,(struct sockaddr *)&indirizzo,sizeof(indirizzo))<strlen(argv[3])){
    		printf("Errore durante l'invio.\n");
    		exit(1);
    	}
    	printf("Messaggio inviato\n");
	
		//Ricezione
   		FD_SET(sock,&cset);
   		if((val=select(FD_SETSIZE,&cset,NULL,NULL,&tval))==-1)
   			err_quit("select() failed");
   		n--;
   		tval.tv_sec=TIMEOUT;
   		if(val==0)
   			printf("Timeout scaduto. Riprovo\n\n");
   	}while(val==0 && n>0);


   	if(FD_ISSET(sock,&cset)){
   		len=sizeof(mittente);
		val=recvfrom(sock,buf_ric,MAX_BUF-1,0,(struct sockaddr *)&mittente, &len);
   		if(val!=-1){
   			buf_ric[val]=0;
   			printf("Il server ha risposto:\n %s\n", buf_ric);
   		}
   		else{
   			switch(errno){
   				case EWOULDBLOCK:
   					printf("No data waiting on a non-blockable socket\n");
   					break;
   				case EBADF:
   				case ENOTSOCK:
   					printf("File descriptor not valid\n");
   					break;
   				case ECONNRESET:
   					printf("Connection closed\n");
   					break;
   				case EINTR:
   					printf("A signal interrupted the function recvfrom()\n");
   					break;
   				case EINVAL:
   					printf("Unbelievable (EINVAL)\n");
   					break;
   				case ENOTCONN:
   					printf("Stream socket not connected\n");
   					break;
   				case EIO:
   					printf("I/O error\n");
   					break;
   				case ENOBUFS:
   					printf("Insufficient resources\n");
   					break;
   				case ENOMEM:
   					printf("Insufficient memory available\n");
   					break;
   				default:
   					break;
   			}
   			err_quit("recvfrom() failed");
   		}
   	} else{
   		printf("Il server non ha risposto. Probabilmente è offline o c'è un problema di rete\n");
   	}
    
	return 0;
}