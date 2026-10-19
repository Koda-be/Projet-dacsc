#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sockets.h"

#define trace(format, ...) printf("[%d] " format "\n", getpid() __VA_OPT__(,) __VA_ARGS__)
#define traceError(errorcode) trace("Program encountered error %d: %s", errorcode, cslib_socketerror_str(errorcode))

int server(const int port);
int client(const char* ip, const int port);
void sigcont_handler(int);
void sigint_handler(int);
void sigsegv_handler(int);

char ready = 0;

int self = 0, 
    other = 0;

int main(int argc, char* argv[])
{
    assert(argc == 3);

    const char  *ipAddress = argv[1],
                *port = argv[2];

    int intPort = atoi(port);


    struct sigaction sigContAction = {.sa_handler = sigcont_handler, .sa_flags = 0};
    sigemptyset(&sigContAction.sa_mask);

    struct sigaction sigIntAction = {.sa_handler = sigint_handler, .sa_flags = 0};
    sigemptyset(&sigIntAction.sa_mask);

    struct sigaction sigSegvAction = {.sa_handler = sigsegv_handler, .sa_flags = 0};
    sigemptyset(&sigSegvAction.sa_mask);

    int sigact_result = 0;

    if((sigact_result = sigaction(SIGCONT, &sigContAction, NULL)))
        trace("SIGCONT armament error: %d", sigact_result);

    else trace ("SIGCONT armed",);


    int parent = getpid();

    trace("forking", );

    int child = fork();

    if (child) 
    {
        trace("Parent program, child: %d", child);

        self = parent;
        other = child;

        server(intPort);
    }

    else
    {
        trace("Child program", );
        other = self;
        self = getpid();
        client(ipAddress, intPort);
    }

    return 0;
}

int server(const int port)
{


    int listenSocket = cslib_serverSocket(port);

    if(listenSocket < 0)
    {
        traceError(listenSocket);

        kill(other, SIGINT);

        exit(0);
    }

    trace("Server listening on port %d with socket %d", port, listenSocket);
    kill(other, SIGCONT);

    int servSocket = cslib_accept(listenSocket);

    if(servSocket < 0)
    {
        traceError(servSocket);

        kill(other, SIGINT);

        exit(0);
    }

    trace("Program connected using socket %d", servSocket);




    return 0;
}

int client(const char* ip, const int port)
{
    while(!ready) pause();

    return 0;
}



// Signal handlers

void SIGCONT_handler(int signal)
{
    trace("Signal received: %d", signal);
    ready = 1;
}

void sigint_handler(int signal)
{
    trace("Signal received: %d, the other program quit unexpectedly", signal);
    exit(0);
}

void sigsegv_handler(int signal)
{
    trace("Signal received: %d, signaling other program and quitting", signal);

    if(other) kill(other, SIGINT);

    exit(0);
}