#ifndef __SOCKET_LIBRARY_H_
#define __SOCKET_LIBRARY_H_

#include <stddef.h>

#define CSLIB_ERROR_SOCKET_CREATION -1
#define CSLIB_ERROR_ADDRESS_INFO_OBTENTION -2
#define CSLIB_ERROR_SOCKET_BINDING -3
#define CSLIB_ERROR_LISTENING_START -4
#define CSLIB_ERROR_ACCEPT_CONNECTION -5
#define CSLIB_ERROR_CONNECT_TO_SERVER -6

/* Creates a socket and starts listening for any incoming connection on port */
int cslib_serverSocket(int port);

/* Creates a service socket from a connection to the listening socket srvrscket */
int cslib_accept(int srvrscket);

/* Creates a socket and connects it to ip:port */
int cslib_clientSocket(char* ip, int port);

/* Sends bffr_size bytes from msg to sckt*/
size_t cslib_send(int sckt, const char* msg, size_t bffr_size);

/* Reads bffr_size bytes from sckt and writes them in memory, then assigns msg to point to said memory */
size_t cslib_receive(int sckt, char* msg, size_t bffr_size);

#endif