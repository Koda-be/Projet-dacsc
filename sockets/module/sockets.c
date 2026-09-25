#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "sockets.h"

int serverSocket(int port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);

    if(s == -1) return CSLIB_ERROR_SOCKET_CREATION;

    struct addrinfo hints;
    struct addrinfo *results;
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    size_t count = 0;
    int cpy = port;
    
    do 
    {
        cpy /= 10;
        ++count;
    }while (cpy != 0);

    char* portstr = (char*) malloc(count+1);
    snprintf(portstr, count+1, "%d", port);

    if (getaddrinfo(NULL,portstr,&hints,&results) != 0)
    {
        close(s);
        freeaddrinfo(results);
        free(portstr);
        return CSLIB_ERROR_ADDRESS_INFO_OBTENTION;
    }

    if(bind(s, results->ai_addr, results->ai_addrlen) < 0)
    {
        freeaddrinfo(results);
        free(portstr);
        return CSLIB_ERROR_SOCKET_BINDING;
    }

    freeaddrinfo(results);
    free(portstr);

    if(listen(s, SOMAXCONN) == -1) 
    {
        close(s);
        return CSLIB_ERROR_LISTENING_START;
    } 

    return s;
}

int cslib_accept(int socket)
{
    int s = accept(socket, NULL, NULL);

    if(s == -1)
    {
        close(s);
        return CSLIB_ERROR_ACCEPT_CONNECTION;
    }

    return s;
}

/* int cslib_clientSocket(char* ip, int port)
{

} */