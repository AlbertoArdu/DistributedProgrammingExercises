#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <rpc/xdr.h>

#include <string.h>

#include <unistd.h>

#include "myLib.h"

#define MAX_BUF 50

//ritorna la lunghezza della stringa letta che finisce con \r\n o \n
//se ritorna max_length probabilmente la linea non è finita
//bloccante se non riceve \n e se non ha letto max_length dati
//ritorna i dati letti fino a quel momento se la connessione cade
int riceviLinea(int sock,char *buffer, size_t max_length){

	static char dati_prec[MAX_BUF];
	static int inizio=0,fine=0, occupati=0; //del buffer circolare
	int k=0;
	int letti_da_sock;

	max_length--;
	
	do{
		//ricevo in dati_prec al massimo [MAX_BUF-(fine-inizio)], 
		//aggiorno "fine"
		if(occupati<MAX_BUF){
			if(fine!=MAX_BUF-1)
				letti_da_sock=recv(sock, &(dati_prec[fine]),(MAX_BUF-fine),0);
			else
				letti_da_sock=recv(sock,&(dati_prec[0]),inizio,0);
		}
		printf("Letti %d byte dal socket\n", letti_da_sock);
		if(letti_da_sock==0)
			return k;
		fine+=letti_da_sock;
		fine%=MAX_BUF;
		occupati+=letti_da_sock;

		for(; occupati>0 && k<max_length;inizio++, k++, occupati--){
			/*
			metti nel buffer tutti i dati vecchi finché:
			-non finiscono i dati vecchi (occupati>0)
			-non arrivo ad un a-capo (dati_prec[inizio]!='\n')
			-non riempio il buffer di input
			*/
			inizio%=MAX_BUF;

			buffer[k]=dati_prec[inizio];
			if(dati_prec[inizio]== '\n' ){
				occupati--;
				inizio++;
				break;
			}
		}

		inizio%=MAX_BUF;
		if(buffer[k]=='\n' || k>=max_length)
			return k++;
	}while(1);
	return -1;
}

int readn (int s, char *ptr, size_t len)
{
	ssize_t nread; size_t nleft;
	for (nleft=len; nleft > 0; ){
		nread=recv(s, ptr, nleft, 0);
	if (nread > 0){
		nleft -= nread;
		ptr += nread;
	}else if (nread == 0) /* conn. closed by party */
		break;
	else /* error */
		return (nread);
	}
	return (len - nleft);
}


int writen(int s, char *ptr, size_t nbytes)
{
	size_t nleft; ssize_t nwritten;
	for (nleft=nbytes; nleft > 0; ){
		nwritten = send(s, ptr, nleft, 0);
		if (nwritten <=0) /* error */
			return (nwritten);
		else {
			nleft -= nwritten;
			ptr += nwritten;
		}
	}
	return (nbytes - nleft);
}