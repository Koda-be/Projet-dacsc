#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "entities.h"
#include "sockets.h"
#include "obep.h"

#define CONFIG_FILE ".server.cfg"
#define PORT_ENCODING "PORT_ENCODING"
#define NTHREAD "NTHREAD"

#define trace(format, ...) printf("[%d] > " format, getpid() __VA_OPT__(,) __VA_ARGS__);

int configure(char* filepath, int* port, int* nbthread);
void* connectionHandler(void* arg);
obep_rqst_t* processRequest(obep_rqst_t* rqst);

int check_employee_existence(const employee_t* empl);

author_t* get_auth_from_db(void);
subject_t* get_subj_from_db(void);

int add_auth_to_db(const author_t* auth);
int add_subj_to_db(const subject_t* subj);
int add_book_to_db(const book_t* book);


int main(void)
{
    int port = 0, nbthread = 0;

    if(configure(CONFIG_FILE, &port, &nbthread) == -1) 
    {
        perror("Couldn't configure through config file");
        exit(0);
    }

    trace("port: %d, nbthread: %d", port, nbthread);

    int lstnscket = cslib_serverSocket(port);

    while(1)
    {
        int srvcscket = cslib_accept(lstnscket);

        connectionHandler(&srvcscket);
    }

    return 0;
}

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

void* connectionHandler(void* arg)
{
    int socket = *(int*) arg;

    obep_msg_t msg;

    cslib_receive(socket, msg.header, OBEP__HEADER_LENGTH);

    msg.msg = malloc(obep_get_msg_size(&msg));

    obep_rqst_t* rqst = obep_get_rqst(&msg);
    free(msg.msg);

    obep_rqst_t* rslt = processRequest(rqst);

    free(rqst->arg); free(rqst);

    obep_msg_t* response = obep_get_msg(rslt);

    free(rslt->arg); free(rslt);

    cslib_send(socket, response->header, OBEP__HEADER_LENGTH);
    cslib_send(socket, response->msg, obep_get_msg_size(response));

    free(response->msg); free(response);

    return NULL;
}