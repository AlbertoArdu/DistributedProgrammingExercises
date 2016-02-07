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

#include "errlib.h"
#include "myLib.h"

#define MAX_BUF 50
#define MAX_CLIENT 10

#define DEBUG

enum stato_client {
	in_attesa,
	primo_invio,		//deve essere inviata la dimensione più la prima parte del file
	in_invio,  			//invio del file iniziato ma non completato
	in_errore,			//devo inviare a questo client la stringa "-ERR\r\n"
	da_scollegare		//devo chiudere la connessione con il client
};

struct s_client{
	int sock;
	struct sockaddr caddr;
	int file_da_inv;
	uint32_t b_tot;
	enum stato_client stato;
};

struct s_client * connessi[MAX_CLIENT];
int n_client;
fd_set cset, wset;
char buf_ric[MAX_BUF], buf_inv[MAX_BUF];
int byte_ric, byte_inv;

void init_socket(int*,struct sockaddr_in *,uint16_t);
void nuova_connessione(int *sock);
void gestisci_client_in_lettura(struct s_client *c);
void gestisci_client_in_scrittura(struct s_client *c);

int main(int argc, char const *argv[]){

	int result, i,j;
	int sock;
	struct sockaddr_in indirizzo;
	uint16_t porta;

	if(argc<2){
		fprintf(stderr, "Formato: %s porta\n", argv[0]);
		exit(1);
	}

	FD_ZERO(&cset);
	FD_ZERO(&wset);

	//Init socket
	porta=htons(atoi(argv[1]));
	init_socket(&sock,&indirizzo,porta);

	FD_SET(sock,&cset);
server:
	//Controllo se c'è un socket pronto
	if((result=select(FD_SETSIZE,&cset,&wset,NULL,NULL)) == -1)
		err_quit("select() fallita");

	if(FD_ISSET(sock,&cset)){ //nuovo socket richiede una connessione
		nuova_connessione(&sock);
		result--;
    }else{
    	FD_SET(sock,&cset);
    }

    //controllo quali socket sono pronti per la lettura
	for(i=0;i<n_client && result>0;i++){
		if(FD_ISSET(connessi[i]->sock,&cset)){
			result--;
			FD_CLR(connessi[i]->sock,&cset);
			gestisci_client_in_lettura(connessi[i]);	
		}else{
			FD_SET(connessi[i]->sock,&cset);
		}
	}
	//controllo quali socket sono pronti per la scrittura
	for(i=0;i<n_client && result>0;i++){
		if(FD_ISSET(connessi[i]->sock,&wset)){
			result--;
			FD_CLR(connessi[i]->sock,&wset);
			gestisci_client_in_scrittura(connessi[i]);
		}
	}

	//libero le risorse dei socket da_scollegare
	//PULIZIA

	for(i=0;i<n_client;i++){
		if(connessi[i]->stato==da_scollegare){
			close(connessi[i]->file_da_inv);
			close(connessi[i]->sock);
			free(connessi[i]);
			connessi[i]=NULL;
		}else if(connessi[i]->stato==in_attesa){
			FD_SET(connessi[i]->sock,&cset);
		}else {
			FD_SET(connessi[i]->sock,&wset);
		}
	}
	for(i=0;i<n_client;i++){
		if(connessi[i]==NULL){
			for(j=i+1;j<n_client;j++){
				if(connessi[j]!=NULL) break;
			}
			connessi[i]=connessi[j];
			connessi[j]=NULL;
			n_client--;
		}
	}

	goto server;
	return 0;
}


void init_socket(int* sock,struct sockaddr_in *indirizzo,uint16_t porta){
	int result;
	indirizzo->sin_family = PF_INET;
	indirizzo->sin_port = porta;
	indirizzo->sin_addr.s_addr = INADDR_ANY;
	*sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(*sock<0)
		err_quit("socket() fallita");
	result = bind(*sock,(struct sockaddr *) indirizzo, sizeof(struct sockaddr));
	if(result<0)
		err_quit("binding fallito");

	listen(*sock,MAX_CLIENT);
}

void nuova_connessione(int *sock){
	int s;
	struct sockaddr addr;
	socklen_t addr_len=sizeof(struct sockaddr);


	#ifdef DEBUG
	printf("In attesa di connessione...\n");
	#endif
	
	if((s=accept(*sock,&addr,&addr_len))>0){
		if(n_client<MAX_CLIENT){
			#ifdef DEBUG
				printf("Connesso con un client\n");
			#endif
			connessi[n_client] = malloc(sizeof(struct s_client));
			connessi[n_client]->sock = s;
			connessi[n_client]->caddr = addr;
			connessi[n_client]->stato = in_attesa;
			connessi[n_client]->file_da_inv = -1;
			n_client++;
			
		}else{
			//Se ho raggiunto MAX_CLIENT chiudo la connessione
			close(s);
		}
			
	}else{
		switch(errno){
			case EWOULDBLOCK:
				err_quit("errore fatale nella accept(): EWOULDBLOCK");
				break;
			case EBADF:
				err_quit("errore fatale nella accept(): EBADF");
				break;
			case EINVAL:
				err_quit("errore fatale nella accept(): EINVAL");
				break;
			case ENOTSOCK:
				err_quit("errore fatale nella accept(): ENOTSOCK");
				break;
			case EOPNOTSUPP:
				err_quit("errore fatale nella accept(): EOPNOTSUPP");
				break;
			//case ECONNABORT:
			case EINTR:
			case EMFILE:
			case ENFILE:
			default:
				;
		}
	}
}

void gestisci_client_in_lettura(struct s_client *c){
	int i;
	struct stat sstr;
	//se ricevo un messaggio mentre invio dati chiudo la connessione
	if(c->stato!=in_attesa){
		c->stato=in_errore;
		return;
	}

	byte_ric=riceviLinea(c->sock,buf_ric,MAX_BUF);
	buf_ric[byte_ric]=0;
	#ifdef DEBUG
		printf("Ricevuto %s\n",buf_ric);
	#endif
	if(strncmp(buf_ric,"GET",3)==0){
		for(i=0;i<byte_ric && (buf_ric[i]!='\r' || buf_ric[i]!='\n');i++);
		buf_ric[i-1]=0;
		buf_ric[1]='.';
		buf_ric[2]='/';
		#ifdef DEBUG
			printf("Cerco di aprire %s\n",&buf_ric[1]);
		#endif

		if(stat(&(buf_ric[1]),&sstr)==-1){
			#ifdef DEBUG
				printf("Fallita stat\n");
			#endif
			c->stato=in_errore;
			return;
		}

		c->b_tot=sstr.st_size;

		c->file_da_inv=open(&buf_ric[1],O_RDONLY);
		if(c->file_da_inv==-1){
			#ifdef DEBUG
				printf("Fallita apertura\n");
			#endif
			c->stato=in_errore;
			return;
		}
		#ifdef DEBUG
			printf("File aperto\n");
		#endif
		c->stato=primo_invio;
	} else{
		c->stato=da_scollegare;
	}
	return;
}

void gestisci_client_in_scrittura(struct s_client *c){
	int n;
	char *buf;

	buf=buf_inv;
	switch(c->stato){
		case in_attesa:
			break;
		case primo_invio:
			strcpy(buf,"+OK\r\n");				//
			*((int*)(buf+5))=htonl(c->b_tot);	//scrivo la dimensione del file in formato rete
			c->stato=in_invio;
			if(writen(c->sock,buf,9)<0)
				c->stato = in_errore;
			
			#ifdef DEBUG
			else
				printf("Inviati %d byte\n",9);
			#endif
			
			break;
		case in_invio:
			n=read(c->file_da_inv,buf,MAX_BUF);
			if(n>0){
				if(writen(c->sock,buf,n)<0){
					c->stato = in_errore;
				}
				#ifdef DEBUG
					printf("Inviati %d byte\n",n);
				#endif
			}else if(n==0){
				c->stato=in_attesa;
				if(close(c->file_da_inv)<0)
					err_quit("Errore fatale in chiusura\n");
				c->file_da_inv=-1;
				FD_CLR(c->sock,&wset);
				FD_SET(c->sock,&cset);

			}else{
				c->stato = in_errore;
			}
			
			break;
		case in_errore:
			strcpy(buf_inv,"-ERR\r\n");
			writen(c->sock,buf,6);
			c->stato = da_scollegare;
			#ifdef DEBUG
				printf("Inviato \"-ERR\"\n");
			#endif
			break;
		case da_scollegare:
			break;
		default:
			err_quit("stato del client invalido");
			break;
	}
}
