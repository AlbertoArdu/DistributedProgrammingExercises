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
	int sock,result, byte_inv;
	socklen_t addr_len, mes_len;
	struct sockaddr_in indirizzo, mittente;
	char buf_ric[MAX_BUF];


	if(argc<2){
		fprintf(stderr, "Formato: %s porta\n", argv[0]);
		exit(1);
	}

    porta=htons(atoi(argv[1]));
    indirizzo.sin_family = PF_INET;
    indirizzo.sin_port = porta;
    indirizzo.sin_addr.s_addr = INADDR_ANY;

    sock = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP);

    result = bind(sock,(struct sockaddr *) &indirizzo, sizeof(indirizzo));
    if(result<0)
    	err_quit("binding fallito");
    printf("Server in ascolto sulla porta %hd\n", ntohs(porta));
    while(mes_len==0){
	    mes_len=recvfrom(sock,buf_ric,MAX_BUF,0,(struct sockaddr *)&mittente, &addr_len);
    }
    if(mes_len<1){
    	err_quit("errore in ricezione");
    }
    buf_ric[mes_len]=0;
    printf("Ricevuto messaggio: %s\n",buf_ric);
    
    printf("Invio in corso...\n");
    byte_inv=sendto(sock,buf_ric,mes_len,0,(struct sockaddr *)&mittente,addr_len);
    
    if(byte_inv<mes_len)
    	printf("Errore di invio\n");
    else
    	printf("Messaggio inviato correttamente: %d\n",byte_inv);

	return 0;
}