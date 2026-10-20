#ifndef __SOCKET_LIBRARY_H_
#define __SOCKET_LIBRARY_H_

#include <stddef.h>
#include <unistd.h>

#define CSLIB_ESOCKET_CREATION -1
#define CSLIB_EADDRESS_INFO_OBTENTION -2
#define CSLIB_ESOCKET_BINDING -3
#define CSLIB_ELISTENING_START -4
#define CSLIB_EACCEPT_CONNECTION -5
#define CSLIB_ECONNECT_TO_SERVER -6

/* Creates a socket and starts listening for any incoming connection on port */
int cslib_serverSocket(const int port);

/* Creates a service socket from a connection to the listening socket srvrscket */
int cslib_accept(int srvrscket);

/* Creates a socket and connects it to ip:port */
int cslib_clientSocket(const char* ip, const int port);

/* Sends bffr_size bytes from msg to sckt*/
ssize_t cslib_send(int sckt, const char* msg, const size_t bffr_size);

/* Reads bffr_size bytes from sckt and writes them in memory, then assigns msg to point to said memory */
ssize_t cslib_receive(int sckt, char* msg, const size_t bffr_size);



const char* cslib_socketerror_str(int errorcode);

#endif