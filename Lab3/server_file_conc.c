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

#include <pthread.h>

#include "errlib.h"
#include "myLib.h"

#define MAX_BUF 50
#define MAX_CLIENT 3

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
	int n_thr;
};

int n_client=0;
char buf_ric[MAX_BUF], buf_inv[MAX_BUF];
int byte_ric, byte_inv;
pthread_t *figli[MAX_CLIENT];
int t_complete[MAX_CLIENT];

void init_socket(int*,struct sockaddr_in *,uint16_t);
void nuova_connessione(int *sock);
void errore_accept(int err);
void errore_pthread_create(int err);
void *gestisci_client(void* arg);
void gestisci_client_in_lettura(struct s_client *c);
void gestisci_client_in_scrittura(struct s_client *c);

int main(int argc, char const *argv[]){

	int i;
	int sock;
	struct sockaddr_in indirizzo;
	uint16_t porta;

	if(argc<2){
		fprintf(stderr, "Formato: %s porta\n", argv[0]);
		exit(1);
	}

	for(i=0;i<MAX_CLIENT;i++){
		t_complete[i]=0;
		figli[i]=NULL;
	}
	//Init socket
	porta=htons(atoi(argv[1]));
	init_socket(&sock,&indirizzo,porta);

server:
	
	
	nuova_connessione(&sock);
	
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


// - accetta la connessione
// - crea e inizializza la struttura s_client
// - sgancia un thread;
void nuova_connessione(int *sock){
	int s, i, ret_val;
	struct sockaddr addr;
	socklen_t addr_len=sizeof(struct sockaddr);
	struct s_client *cl;
	pthread_attr_t attr;

	#ifdef DEBUG
	printf("In attesa di connessione...\n");
	#endif
	
	
	if((s=accept(*sock,&addr,&addr_len))>0){
		//libero le risorse dei thread completi
		//PULIZIA

		for(i=0;i<MAX_CLIENT;i++){
			if(t_complete[i] && figli[i]!= NULL){
				pthread_join(*figli[i],NULL);
				free(figli[i]);
				figli[i]=NULL;
				n_client--;
				#ifdef DEBUG
					printf("Thread %d terminato\n",i);
				#endif
			}
		}
		
		if(n_client<MAX_CLIENT){
			
			for(i=0;i<MAX_CLIENT && figli[i]!= NULL; i++);

			figli[i]=malloc(sizeof(pthread_t));

			cl=malloc(sizeof(struct s_client));
			if(cl==NULL)
				err_quit("errore in malloc(): nuova_connessione()");
			cl->sock = s;
			cl->caddr = addr;
			cl->stato = in_attesa;
			cl->file_da_inv = -1;
			cl->n_thr = i;
			t_complete[i] = 0;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

			if((ret_val=pthread_create(figli[i],&attr,gestisci_client,(void*)cl))!=0){
				free(cl);
				free(figli[i]);
				figli[i]=NULL;
				errore_pthread_create(ret_val);
			}
			else{
				n_client++;
			}
		}else{
			//Se ho raggiunto MAX_CLIENT chiudo la connessione
			close(s);
		}
			
	}else{
		errore_accept(errno);
		
	}
}

void gestisci_client_in_lettura(struct s_client *c){
	int i;
	struct stat sstr;
	//se ricevo un messaggio mentre invio dati chiudo la connessione
	if(c->stato!=in_attesa){
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
				printf("Inviati %d byte (+OKdim)\n",9);
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

void errore_accept(int err){
	char *mess;
	switch(errno){
		case EWOULDBLOCK:
			mess = "errore fatale nella accept(): EWOULDBLOCK";
			break;
		case EBADF:
			mess = "errore fatale nella accept(): EBADF";
			break;
		case EINVAL:
			mess = "errore fatale nella accept(): EINVAL";
			break;
		case ENOTSOCK:
			mess = "errore fatale nella accept(): ENOTSOCK";
			break;
		case EOPNOTSUPP:
			mess = "errore fatale nella accept(): EOPNOTSUPP";
			break;
		//case ECONNABORT:
		case EINTR:
		case EMFILE:
		case ENFILE:
		default:
			mess = "errore fatale nella accept()";
	}
	err_quit(mess);
}

void errore_pthread_create(int err){
	char *mess;
	switch(errno){
			case EAGAIN:
				mess = "errore fatale nella accept(): EAGAIN -> mancano le risorse";
				break;
			case EINVAL:
				mess = "errore fatale nella accept(): EINVAL";
				break;
			case EPERM:
				mess = "errore fatale nella accept(): EPERM -> permessi insufficienti";
			default:
				mess = "errore fatale nella accept()";
		}
		err_quit(mess);
}


//arg punta alla struttura s_client che deve essere gestito;
//il thread diventa il possessore di questa locazione di memoria;
void *gestisci_client(void* arg){
	struct s_client *cl;
	pthread_t self;

	cl=(struct s_client *)arg;
	self=pthread_self();
	#ifdef DEBUG
		printf("Sono il thread %d\n",(int)self);
	#endif
thread_connesso:
	gestisci_client_in_lettura(cl);
	gestisci_client_in_scrittura(cl);

	if(cl->stato==da_scollegare){
		close(cl->file_da_inv);
		close(cl->sock);
		t_complete[cl->n_thr]=1;
		free(cl);
		pthread_exit((void*)0);
	}

	goto thread_connesso;
}