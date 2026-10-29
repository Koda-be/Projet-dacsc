#include <stdio.h>
#include <threads.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include "sockets.h"

static int cslib_getaddrinfo(const char* host, const int port, const int hint_flags, struct addrinfo** results);

int cslib_serverSocket(const int port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);

    if(s == -1)
    {
        errno = CSLIB_ESOCKET_CREATION;
        return -1;
    } 

    struct addrinfo *results;


    if(cslib_getaddrinfo(NULL, port, AI_PASSIVE,&results) != 0)
    {
        close(s);
        freeaddrinfo(results);
        errno = CSLIB_EADDRESS_INFO_OBTENTION;
        return -1;
    }

    if(bind(s, results->ai_addr, results->ai_addrlen) < 0)
    {
        freeaddrinfo(results);
        errno = CSLIB_ESOCKET_BINDING;
        return -1;
    }
    freeaddrinfo(results);

    if(listen(s, SOMAXCONN) == -1) 
    {
        close(s);
        errno = CSLIB_ELISTENING_START;
        return -1;
    } 

    return s;
}

int cslib_accept(int srvrscket)
{
    int s = accept(srvrscket, NULL, NULL);

    if(s == -1)
    {
        close(s);
        errno = CSLIB_EACCEPT_CONNECTION;
        return -1;
    }

    return s;
}

int cslib_clientSocket(const char* ip, const int port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo* results;

    if(cslib_getaddrinfo(ip, port, 0, &results) != 0)
    {
        freeaddrinfo(results);
        errno = CSLIB_EADDRESS_INFO_OBTENTION;
        return -1;
    }

    if(connect(s, results->ai_addr, results->ai_addrlen) == -1)
    {
        freeaddrinfo(results);
        errno = CSLIB_ECONNECT_TO_SERVER;
    }

    return s;
}

ssize_t cslib_send(int sckt, const void* msg, const size_t bffr_size)
{
    size_t total = 0;

    while (total < bffr_size)
    {
        ssize_t n = send(
            sckt,
            (const char*) msg + total,
            bffr_size - total,
            0);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
        {
            errno = EPIPE;
            return -1;
        }

        total += (size_t)n;

    }

    return (ssize_t)total;
}

ssize_t cslib_receive(int sckt, void* msg, const size_t bffr_size)
{
    size_t total = 0;

    while (total < bffr_size)
    {
        ssize_t n = recv(
            sckt,
            (char*)msg + total,
            bffr_size - total,
            0);

        if (n == 0)
            return 0;

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        total += (size_t)n;
    }

    return (ssize_t)total;
}


const char* cslib_socketerror_str(const int errorcode)
{
    switch(errorcode)
    {
        case CSLIB_ESOCKET_CREATION: return "could not create socket";
        case CSLIB_EADDRESS_INFO_OBTENTION: return "could not obtain address informations";
        case CSLIB_ESOCKET_BINDING: return "could not bind socket";
        case CSLIB_ELISTENING_START: return "could not start listening loop";
        case CSLIB_EACCEPT_CONNECTION: return "could not accept connection";
        case CSLIB_ECONNECT_TO_SERVER: return "connection failed";
        default: return strerror(errorcode);
    }
}

// Internal functions

static int cslib_getaddrinfo(const char* host, const int port, const int hint_flags, struct addrinfo** results)
{
    struct addrinfo hints;
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = hint_flags | AI_NUMERICSERV;

    size_t count = 0;
    int cpy = port;

    const char* hoststr;
    char* iptostr = NULL;

    if(hint_flags & AI_NUMERICHOST && host != NULL)
    {
        size_t strsize = strlen("000.000.000.000") + 1;
        iptostr = (char*) malloc(strsize);

        snprintf(iptostr, strsize, "%hhd.%hhd.%hhd.%hhd", (int) host[0], (int) host[1], (int) host[2], (int) host[3]);
        hoststr = iptostr;
    }
    else hoststr = host;

    do 
    {
        cpy /= 10;
        ++count;
    }while (cpy != 0);

    char* portstr = (char*) malloc(count+1);
    snprintf(portstr, count+1, "%d", port);


    int retval = getaddrinfo(hoststr,portstr,&hints,results);

    free(iptostr);
    free(portstr);
    
    return retval;
}