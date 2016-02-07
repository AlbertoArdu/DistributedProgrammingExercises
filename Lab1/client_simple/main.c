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
//#include "sockwrap.h"

#define MAX_BUF 50

int riceviLinea(int socket,char *buffer, size_t size); //Ritorna i byte letti

int main(int argc, char **argv){

    uint16_t tport_n;
    uint16_t n1, n2, somma;
    int sock;
    struct sockaddr_in indirizzo;
    int result, prova=3, da_inviare, inviati, ricevuti,i;
    char ip[30], buf_inv[MAX_BUF+1], buf_ric[MAX_BUF+1];

    if(argc<3){
        printf("Formato: %s ip porta\n",argv[0]);
        exit(0);
    }

    if(!inet_pton(AF_INET,argv[1], &(indirizzo.sin_addr)))
        err_quit("indirizzo non valido");

    tport_n=htons(atoi(argv[2]));
    indirizzo.sin_family = PF_INET;
    indirizzo.sin_port = tport_n;

    sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock <= 0)
        err_quit("socket() failed");

    inet_ntop(AF_INET,&(indirizzo.sin_addr),ip,INET_ADDRSTRLEN);
    printf("Mi sto connettendo alla porta %d dell'indirizzo %s\n",ntohs(tport_n),ip);
    result = connect(sock,(struct sockaddr*)&indirizzo,sizeof(indirizzo));
    if(result == -1)
        err_quit("connect() failed");

    printf("Connesso.\n");
    //Input da tastiera:
    printf("Scrivi due numeri di cui vuoi sapere la somma:\n");
    
    do{
    	printf("n1 - ?");
    	prova--;
    }while(scanf("%hd",&n1)<1 && prova>0);

    if(prova==0){
    	printf("Hai provato troppe volte\n");
    	exit(-1);
    }
    
    prova=3;

    do{
    	printf("n2 - ?");
		prova--;
	}while(scanf("%hd",&n2)<1 && prova>0);

	if(prova==0){
    	printf("Hai provato troppe volte\n");
    	exit(-1);
    }
    //Fine input
    //Scrivo il buffer da inviare
    sprintf(buf_inv,"%hd %hd\r\n",n1,n2);

    // Send buffer
    da_inviare=strlen(buf_inv);
    inviati=0;
    //La prossima riga è un po' un casino ma dovrebbe andare
    while((inviati=send(sock,&buf_inv[inviati],da_inviare,0))<da_inviare)
    	da_inviare-=inviati;

    //ricevi risultato
    //se l'ultimo carattere non è un \n considero un errore
    ricevuti=riceviLinea(sock,buf_ric,MAX_BUF);
	if(buf_ric[ricevuti]!='\n'){;
		fputs("Errore! Dati ricevuti in formato errato (manca Line Feed)",stderr);
		exit(-1);
    }

    if(sscanf(buf_ric,"%hd",&somma)<1){
    	i=0;
    	while(buf_ric[i]!='\n' && i<MAX_BUF)i++;
    	buf_ric[i+1]=0;
    	printf("Messaggio di errore dal Server:\n");
    	printf("%s",buf_ric);
    }else{
    	printf("%hd + %hd = %hd\n",n1,n2,somma );
    }

    return 0;
}


//ritorna la lunghezza della stringa letta che finisce con \r\n o \n
//se ritorna max_length probabilmente la linea non è finita
//bloccante se non riceve \n e se non ha letto max_length dati
int riceviLinea(int sock,char *buffer, size_t max_length){

	static char dati_prec[MAX_BUF];
	static int inizio=0,fine=0; //del buffer circolare
	int i, k=0, flag;
	int letti_da_sock;

	do{
	for(i=inizio; i!=fine && (buffer[k]=dati_prec[i])!= '\n' && k<max_length;i++, k++){
		/*
		metti nel buffer tutti i dati vecchi finché:
		-non finiscono i dati vecchi (i<fine)
		-non arrivo ad un a-capo (dati_prec[i]!='\n')
		-non riempio il buffer di input
		*/
		i%=MAX_BUF; //buffer circolare;
	}

	inizio+=i;
	inizio%=MAX_BUF;
	if(buffer[k]=='\n' || k>=max_length)
		return k;

	//ricevo in dati_prec al massimo [MAX_BUF-(fine-inizio)], 
	//aggiorno "fine"
	if((MAX_BUF-fine+inizio)>0){
		letti_da_sock=recv(sock, &dati_prec[fine],(MAX_BUF-fine+inizio),0);
	}
	fine+=letti_da_sock;
	fine%=MAX_BUF;
	}while(1);
	return -1;
}