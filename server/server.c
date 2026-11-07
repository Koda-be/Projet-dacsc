#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "sockets.h"
#include "obep.h"

#define CONFIG_FILE ".server.cfg"
#define PORT_ENCODING "PORT_ENCODING"
#define NTHREAD "NTHREAD"

int configure(char* filepath, int* port, int* nbthread)
{
    int configfd;

    if((configfd = open(filepath, O_RDONLY)) == -1) return -1;

    size_t fsize = (size_t) lseek(configfd, 0, SEEK_END);
    char* strfile = (char*) calloc( fsize+ 1, sizeof(char));

    lseek(configfd, 0, SEEK_SET);
    if(read(configfd, strfile, fsize) == -1) return -1;
    
    char *token = NULL, *context = NULL;

    for(char* str1 = strfile; ; str1 = NULL)
    {
        if((token = strtok_r(str1, "\n", &context)) == NULL) break;

        char *subtoken, *subcontext;
        subtoken = strtok_r(token, ":=", &subcontext);

        int* param;
        if(!strcmp(subtoken, PORT_ENCODING)) param = port;
        else if(!strcmp(subtoken, NTHREAD)) param = nbthread;
        else goto inval_value;

        subtoken = strtok_r(NULL, ":=", &subcontext);

        sscanf(subtoken, "%d", param);
        if(*param <= 0) goto inval_value;
    }

    free(strfile);

    if(close(configfd) == -1) return -1;

    return 0;

inval_value:
    free(strfile);
    errno = EINVAL;
    return -1;
}

int main(void)
{
    int port = 0, nbthread = 0;

    if(configure(CONFIG_FILE, &port, &nbthread) == -1) 
    {
        perror("Couldn't configure through config file");
        exit(0);
    }

    printf("port: %d, nbthread: %d", port, nbthread);

    return 0;
}