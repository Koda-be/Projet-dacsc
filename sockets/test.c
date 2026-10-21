#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "sockets.h"

#define trace(format, ...) printf("[%d] %s: " format "\n", getpid(), header __VA_OPT__(,) __VA_ARGS__)
#define traceError(errorcode) trace("Program encountered error %d: %s", errorcode, cslib_socketerror_str(errorcode))

int server(const int port);
int client(const char* ip, const int port);
void sigcont_handler(int);
void sigint_handler(int);
void sigsegv_handler(int);

void abort_test(int errorcode);

const char* header = "";
const char* servHead = "[Server]";
const char* clientHead = "[Client]";

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

    int childpid = fork();

    if (childpid) 
    {
        header = servHead;
        trace("Parent program, child: %d", childpid);

        self = parent;
        other = childpid;

        server(intPort);

        int childReturnCode;
        wait(&childReturnCode);
    }

    else
    {
        header = clientHead;
        trace("Child program", );

        other = parent;
        self = getpid();
        
        client(ipAddress, intPort);
    }

    trace("Program terminated, exiting", );

    return 0;
}

int server(const int port)
{
    int listenSocket = cslib_serverSocket(port);

    if(listenSocket < 0) abort_test(listenSocket);

    trace("Listening on port %d with socket %d", port, listenSocket);
    kill(other, SIGCONT);

    int servSocket = cslib_accept(listenSocket);

    if(servSocket < 0) abort_test(servSocket);

    trace("Program connected using socket %d", servSocket);


    ssize_t readbytes;

    size_t msgSize = 0;

    char* msg = NULL;

    while((readbytes = cslib_receive(servSocket, &msgSize, sizeof(size_t))))
    {
        trace("message size: %lu", msgSize);
        msg = realloc(msg, msgSize+1); // add 1 to msgSize to add '\0' at the end of msg

        msg[msgSize] = 0;

        readbytes = cslib_receive(servSocket, msg, msgSize);

        trace("Received: %s", msg);
    }

    trace("Connection terminated",);

    free(msg);

    kill(other, SIGCONT);

    return 0;
}

int client(const char* ip, const int port)
{
    while(!ready) pause();

    ready = 0;

    int sckt = cslib_clientSocket(ip, port);

    if(sckt < 0) abort_test(sckt);

    return 0;
}



// Signal handlers

void sigcont_handler(int signal)
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


void abort_test(int errorcode)
{
    traceError(errorcode);

    kill(other, SIGINT);

    exit(0);
}