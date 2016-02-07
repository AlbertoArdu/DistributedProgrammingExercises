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

struct client_id
{
    struct in_addr ip_addr;
    int n;
};

int main(int argc, char const *argv[]){
	uint16_t porta;
	int sock,result, byte_inv, mes_len;
	socklen_t addr_len;
	struct sockaddr_in indirizzo, mittente;
	char buf_ric[MAX_BUF];

    struct client_id elenco[10];
    int testa = 0,coda=0, i;

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
server:
    addr_len=sizeof(mittente);
    mes_len=recvfrom(sock,buf_ric,MAX_BUF,MSG_WAITALL,(struct sockaddr *)&mittente, &addr_len);


    //Controllo dell'ip
    for(i=testa;i<coda;i++){
        if(elenco[i].ip_addr.s_addr == ((struct sockaddr_in)mittente).sin_addr.s_addr){
            elenco[i].n++;
            if(elenco[i].n>5){
                elenco[i].n=6;
                goto server;  //Non rispondo
            }
            break;
        }
        i%=10;   //buffer circolare;
    }

    if(i==coda){
        elenco[coda].ip_addr.s_addr=mittente.sin_addr.s_addr;
        elenco[coda].n=1;
        coda++;
        coda%=10;
        if(testa==coda)
            testa++;
        //Se la coda è piena elimino il primo elemento (più vecchio);
    }
    if(mes_len<1){
    	err_quit("errore in ricezione");
    }
    buf_ric[mes_len]=0;
    printf("Ricevuto messaggio: %s\n",buf_ric);
    
    printf("Invio in corso...\n");
    byte_inv=sendto(sock,buf_ric,mes_len,0,(struct sockaddr *)&mittente,addr_len);
    
    if(byte_inv<mes_len)
    	printf("Errore di invio: %d - %d\n", byte_inv,mes_len);
    else
    	printf("Messaggio inviato correttamente: %d\n",byte_inv);


    goto server;
	return 0;
}