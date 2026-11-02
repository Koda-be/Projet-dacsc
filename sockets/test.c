#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "sockets.h"

#define trace(format, ...) printf("[%d] %s: " format "\n", getpid(), header __VA_OPT__(,) __VA_ARGS__)
#define traceError(errorcode) trace("Program encountered error %d: %s", errorcode, cslib_socketerror_str(errorcode))
#define MAXINPUTLENGTH 128

int server(const int port);
int client(const char* ip, const int port);
void sigcont_handler(int);
void sigint_handler(int);
void sigsegv_handler(int);
void sckt_cleanup(void);

void abort_test(int errorcode);

const char* header = "";
const char* servHead = "[Server]";
const char* clientHead = "[Client]";

int listenSocket = -1,
    servSocket = -1,
    sckt = -1;

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

    if((sigact_result = sigaction(SIGINT, &sigIntAction, NULL)))
        trace("SIGINT armament error: %d", sigact_result);

    else trace ("SIGINT armed",);

    if((sigact_result = sigaction(SIGSEGV, &sigSegvAction, NULL)))
        trace("SIGSEGV armament error: %d", sigact_result);

    else trace ("SIGSEGV armed",);

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
    listenSocket = cslib_serverSocket(port);

    if(listenSocket < 0) abort_test(errno);

    trace("Listening on port %d with socket %d", port, listenSocket);
    kill(other, SIGCONT);

    servSocket = cslib_accept(listenSocket);

    if(servSocket < 0) abort_test(errno);

    trace("Program connected using socket %d", servSocket);

    ssize_t readbytes;

    size_t msgSize = 0;

    char* msg = NULL;

    while((readbytes = cslib_receive(servSocket, &msgSize, sizeof(size_t))) > 0)
    {
        trace("message size: %lu", msgSize);
        msg = realloc(msg, msgSize+1); // add 1 to msgSize to add '\0' at the end of msg

        msg[msgSize] = 0;

        readbytes = cslib_receive(servSocket, msg, msgSize);

        trace("Received: %s", msg);

        kill(other, SIGCONT);
    }

    trace("Connection terminated",);

    free(msg);

    kill(other, SIGCONT);

    return 0;
}

int client(const char* ip, const int port)
{
    trace("in client function", );
    while(!ready) 
    {
        trace("ready = %d", ready);
        pause();
    }

    ready = 0;

    sckt = cslib_clientSocket(ip, port);

    if(sckt < 0) abort_test(errno);

    char input[MAXINPUTLENGTH];

    trace("Input message (exit/e/q to quit):\n", );

    void* msg = NULL;
    size_t msgSize;

    while(1)
    {
        ready = 0;
        printf("[%d] %s: > ", getpid(), header ); // Can't use trace bc of new line

        if(fgets(input, MAXINPUTLENGTH, stdin) == NULL)
        {
            perror("Error during message input");
            abort_test(0);
        }

        if(!strcmp(input, "q\n") || !strcmp(input, "e\n") || !strcmp(input, "exit\n")) break;

        msgSize = strlen(input);
        (input[msgSize-1] == '\n') ? input[msgSize-- -1] = '\0' : 0;

        msg = realloc(msg,sizeof(msgSize) + msgSize);

        size_t* head = (size_t*) msg;
        *head = msgSize;

        char* msgStr = (char*) (head + 1);

        memcpy(msgStr, input, msgSize);

        trace("Sending %lu bytes", (*head)+sizeof(size_t));

        cslib_send(sckt, msg, msgSize + sizeof(msgSize));

        while(!ready) pause();
    }

    ready = 0;    
    close(sckt);

    while(!ready) pause();

    free(msg);

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
    sckt_cleanup();
    exit(0);
}

void sigsegv_handler(int signal)
{
    trace("Signal received: %d, signaling other program and quitting", signal);

    if(other) kill(other, SIGINT);

    sckt_cleanup();

    exit(0);
}


void abort_test(int errorcode)
{
    if(errorcode) traceError(errorcode);
    perror("");

    kill(other, SIGINT);

    sckt_cleanup();

    exit(0);
}

void sckt_cleanup(void)
{
    if(servSocket != -1) close(servSocket);
    if(listenSocket != -1) close(listenSocket);
    if(sckt != -1) close(sckt);
}