#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "sockets.h"
#include "obep.h"

int main(void)
{
    int configfd;
    if((configfd = open(".server.conf", O_RDONLY)) == -1)
    {
        perror("Error opening config file");
        exit(0);
    }

    size_t fsize = (size_t) lseek(configfd, 0, SEEK_END);
    char* strfile = (char*) calloc( fsize+ 1, sizeof(char));

    lseek(configfd, 0, SEEK_SET);
    if(read(configfd, strfile, fsize) == -1)
    {
        perror("Error while trying to read config file");
        exit(0);
    }
    int port = 0;
    if(sscanf(strfile, "PORT_ENCODING:%d ", &port) != 1)
    {
        perror("Invalid config file");
        exit(0);
    }

    free(strfile);

    if(close(configfd) == -1)
    {
        perror("Error while closing config file");
        exit(0);
    }

    
}