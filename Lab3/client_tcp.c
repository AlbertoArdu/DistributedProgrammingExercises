#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <rpc/xdr.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>
#include <time.h>

#include "myLib.h"
//#include "sockwrap.h"

#define MAX_BUF 50
#define MAX_FIGLI 10

//Creo un figlio ogni volta che voglio connettermi con il server
//Il padre gestisce l'interfaccia utente;
enum stato_figlio
{
	in_corso
	completato
};

struct figlio
{
	int pid;
	char *nomefile;
	enum stato_figlio stato;
};


void ottieni_file(void);

int main(int argc, char **argv){

    uint16_t tport_n;
    uint32_t dim;
    int sock, f_out, n_figli=0, pid_figlio, *status;
    struct figlio figli[MAX_FIGLI];

    struct sockaddr_in indirizzo;
    int result, da_inviare, inviati, ricevuti, scritti, i, n_file;
    char ip[INET_ADDRSTRLEN], buf_inv[MAX_BUF+1], buf_ric[MAX_BUF+1], **nomefile, linea[MAX_BUF];

    if(argc<4){
        printf("Formato: %s ip porta numero_massimo_di_file\n",argv[0]);
        exit(0);
    }

    if(!inet_pton(AF_INET,argv[1], &(indirizzo.sin_addr)))
        err_quit("indirizzo non valido");

    n_file=atoi(argv[3]);
    nomefile=(char**)malloc(n_file*sizeof(void*));
    if(nomefile==NULL)
        err_quit("Ouy of memory");

    tport_n=htons(atoi(argv[2]));
    indirizzo.sin_family = PF_INET;
    indirizzo.sin_port = tport_n;

    sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0)
        err_quit("socket() failed");
    
    //Interfaccia utente
    
    printf("Scrivi:\n - Nome del file da scaricare\n - Q (completa i download in corso e termina)\n - A (abortisce tutti i download e esce\n");
    printf("?- \n");	
    i=0;

    do{    
        fgets(linea,MAX_BUF-10,stdin);
        if(strcmp(linea,"Q\n")==0)
            break;
        if(strcmp(linea,"A\n")==0){
        	uccidi_figli(figli);
        	break;
        }
        if(n_figli>=MAX_FIGLI){
        	printf("Troppi download in corso\n");
        	printf("Attendere per il completamento di almeno un file\n");
        	pid_figlio = wait(status);
        	n_figli--;
        	for(i=0;i<n_figli;i++){
        		if(figli[i].pid==pid_figlio){
        			figli[i].stato = completato;
        			if(WIFEXITED(status)){
        				if(WEXITSTATUS(status)==1)
        					printf("Download di %s non riuscito\n", figli[i].nomefile);
        			
        				else
	        				printf("Download del file %s completato con successo\n", figli[i].nomefile);
        			}else{
        				printf("Download del file %s completato con successo\n", figli[i].nomefile);

        			}
        			free(figli[i].nomefile);
        			break;
        		}
        	}
        }else{
        	for (i = 0; i < n_figli; ++i)
        	{
        		if(figli[i].stato==completato)
        			break;
        	}
        }

        figli[i].nomefile=malloc(strlen(linea)+2);
        strcpy(figl[i].nomefile,linea);
        figli[i].nomefile[strlen(nomefile[i])-1]=0;  //rimuovo il carattere '\n'

        figli[i].pid=fork();
    	
    	if(figli[i].pid<0)
    		err_quit("Impossibile creare nuovo figlio");
    	if(figli[i].pid==0){
    		ottieni_file();
		}

		//Pulizia
		for(i=0;i<n_figli; i++){
			pid_figlio=waitpid(figli[i].pid,status,WNOHANG);
			if(pid_figlio!=0){
				figli[i].stato = completato;
    			if(WIFEXITED(status)){
    				if(WEXITSTATUS(status)==1)
    					printf("Download di %s non riuscito\n", figli[i].nomefile);
    			
    				else
        				printf("Download del file %s completato con successo\n", figli[i].nomefile);
    			}else{
    				printf("Download del file %s completato con successo\n", figli[i].nomefile);

    			}
    			free(figli[i].nomefile);
			}
		}

	}while(i<n_file);


    
    return 0;
}

void ottieni_file(void){

	n_file=i;
    inet_ntop(AF_INET,&(indirizzo.sin_addr),ip,INET_ADDRSTRLEN);

    printf("Mi sto connettendo alla porta %d dell'indirizzo %s\n",ntohs(tport_n),ip);
    result = connect(sock,(struct sockaddr*)&indirizzo,sizeof(indirizzo));
    if(result == -1)
        err_quit("connect() failed");

    printf("Connesso.\n");
    
    //Scrivo il buffer da inviare
    sprintf(buf_inv,"GET%s\r\n",nomefile[i]);
    // Send buffer
    da_inviare=strlen(buf_inv);
    inviati=writen(sock,buf_inv,da_inviare);

    ricevuti=readn(sock,buf_ric,5);
        
    if(buf_ric[0]=='+'){
        ricevuti=readn(sock,buf_ric,4);
        if(ricevuti==4)
            dim=ntohl(*((int*)buf_ric));
        else{
            printf("Errore sulla ricezione della dimensione del file %s\n",nomefile[i]);
        }
        f_out = open(nomefile[i],O_WRONLY | O_CREAT); //Apri file
        if(f_out>0){
            while(dim>MAX_BUF-1){
                ricevuti=readn(sock,buf_ric,MAX_BUF-1);
                scritti=write(f_out,buf_ric,ricevuti);
                if(scritti<ricevuti)
                    err_quit("errore nella write(). File %s",nomefile[i]);
                dim-=ricevuti;
            }
            ricevuti=readn(sock,buf_ric,dim);
            scritti=write(f_out,buf_ric,ricevuti);
            if(scritti < ricevuti)
                err_quit("errore nella write()");
            close(f_out);    
        }
    }else {
    	exit(0);
    }

}