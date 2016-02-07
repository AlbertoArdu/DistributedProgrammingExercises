#include "errlib.h"

int riceviLinea(int socket,char *buffer, size_t size); //Ritorna i byte letti
int readn (int s, char *ptr, size_t len);
int writen(int s, char *ptr, size_t nbytes);
