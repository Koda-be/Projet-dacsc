#ifndef __SOCKET_LIBRARY_H_
#define __SOCKET_LIBRARY_H_

#include <stddef.h>

#define CSLIB_ERROR_SOCKET_CREATION -1
#define CSLIB_ERROR_ADDRESS_INFO_OBTENTION -2
#define CSLIB_ERROR_SOCKET_BINDING -3
#define CSLIB_ERROR_LISTENING_START -4
#define CSLIB_ERROR_ACCEPT_CONNECTION -5
#define CSLIB_ERROR_CONNECT_TO_SERVER -6

int cslib_serverSocket(int port);
int cslib_accept(int socket);
int cslib_clientSocket(char* ip, int port);
size_t cslib_send(const char* msg);
size_t cslib_receive(char** msg, size_t num);

#endif