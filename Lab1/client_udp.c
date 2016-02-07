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

#include "errlib.h"

#define MAX_BUF 50

int main(int argc, char const *argv[]){
	uint16_t porta;
	int sock;
	socklen_t len;
	struct sockaddr_in indirizzo, mittente;
	char buf_ric[MAX_BUF];


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
    printf("Sto inviando la stringa al server...\n");
    if(sendto(sock,argv[3],strlen(argv[3]),0,(struct sockaddr *)&indirizzo,sizeof(indirizzo))<strlen(argv[3])){
    	printf("Errore durante l'invio.\n");
    	exit(1);
    }
    printf("Messaggio inviato\n");
    
    recvfrom(sock,buf_ric,MAX_BUF,0,(struct sockaddr *)&mittente, &len);

    printf("Il server ha risposto:\n %s\n", buf_ric);

	return 0;
}