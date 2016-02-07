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


int main(int argc, char **argv){

    uint16_t tport_n;
    uint32_t dim;
    int sock, f_out;
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
    
    printf("Scrivi i nomi dei file da scaricare (\"stop\" per terminare)\n");
    i=0;
    do{
        fgets(linea,MAX_BUF-10,stdin);

        if(strncmp(linea,"stop",4)==0)
            break;
        nomefile[i]=malloc(strlen(linea)+2);
        strcpy(nomefile[i],linea);
        nomefile[i][strlen(nomefile[i])-1]=0;  //rimuovo il carattere '\n'
        i++;
    }while(i<n_file);
    n_file=i;
    inet_ntop(AF_INET,&(indirizzo.sin_addr),ip,INET_ADDRSTRLEN);

    printf("Mi sto connettendo alla porta %d dell'indirizzo %s\n",ntohs(tport_n),ip);
    result = connect(sock,(struct sockaddr*)&indirizzo,sizeof(indirizzo));
    if(result == -1)
        err_quit("connect() failed");

    printf("Connesso.\n");
    
    //Scrivo il buffer da inviare
    for(i=0;i<n_file;i++){
        sprintf(buf_inv,"GET%s\r\n",nomefile[i]);
        // Send buffer
        da_inviare=strlen(buf_inv);
        inviati=writen(sock,buf_inv,da_inviare);
        if(inviati<0){
            printf("Errore nell'invio del nome del file %s\n", nomefile[i]);
            printf("Provo a riconnettermi\n");
            close(sock);
            sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            
            printf("Mi sto connettendo alla porta %d dell'indirizzo %s\n",ntohs(tport_n),ip);
            result = connect(sock,(struct sockaddr*)&indirizzo,sizeof(indirizzo));
            if(result == -1)
                err_quit("connect() failed");

            printf("Connesso.\n");
        }

        //ricevi risultato
        //se l'ultimo carattere non è un \n considero un errore
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
        }else if(buf_ric[0]=='-'){
            printf("Ricevuto '-ERR' per il file %s.\n Provo con il successivo, ma prima devo riconnettermi.\n", nomefile[i]);
            printf("Mi sto connettendo alla porta %d dell'indirizzo %s\n",ntohs(tport_n),ip);
            result = connect(sock,(struct sockaddr*)&indirizzo,sizeof(indirizzo));
            if(result == -1)
                err_quit("Connessione rifiutata");

            printf("Connesso.\n");
        }else{
            printf("Errore nella comunicazione\n");
            exit(-1);
        }

    }
    
    strcpy(buf_inv,"QUIT\r\n");
    writen(sock,buf_inv,6);

    return 0;
}