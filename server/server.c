#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <mysql.h>
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

int get_auth_from_db(author_t** array);
int get_subj_from_db(subject_t** array);

int add_auth_to_db(const author_t* auth);
int add_subj_to_db(const subject_t* subj);
int add_book_to_db(const book_t* book);

int loggedIn;

int main(void)
{
    int port = 0, nbthread = 0;

    if(configure(CONFIG_FILE, &port, &nbthread) == -1) 
    {
        perror("Couldn't configure through config file");
        exit(0);
    }

    trace("port: %d, nbthread: %d", port, nbthread);

    MYSQL* mysqlConnection = mysql_init(NULL);
    if(mysql_real_connect(mysqlConnection, "localhost", "DACSC", "DACSC#CassLioB38@", "DACSC", 0,0,0) == NULL) 
    {
        trace("Error with mysql connection: %s", mysql_error(mysqlConnection));
        mysql_close(mysqlConnection);
        exit(0);
    }

    int lstnscket = cslib_serverSocket(port);

    while(1)
    {
        loggedIn = 0;
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

    while(cslib_receive(socket, msg.header, OBEP__HEADER_LENGTH) != -1)
    {    
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
    }

    return NULL;
}



obep_rqst_t* processRequest(obep_rqst_t* rqst)
{
    obep_rqst_t* rslt = (obep_rqst_t*) malloc(sizeof(obep_rqst_t));

    rslt->rqst_type = rqst->rqst_type;
    rslt->msg_type = obep_ans;

    if(!loggedIn && rslt->rqst_type != obep_login)
    {
        rslt->rqst_rslt = obep_failure;
        rslt->argc = 0;
        rslt->arg = NULL;
    }

    switch(rqst->rqst_type)
    {
        case obep_login:
            rslt->argc = 0;
            rslt->arg = NULL;

            if(check_employee_existence(rqst->arg))
            {
                loggedIn = 1;
                rslt->rqst_rslt = obep_success;
            }
            else 
            {
                rslt->rqst_rslt = obep_failure;
                loggedIn = 0;
            }

            break;

        case obep_logout:
            loggedIn = 0;
            break;

        case obep_get_auth:
            rslt->argc = get_auth_from_db((author_t**) &(rslt->arg));
            break;

        case obep_get_subj:
            rslt->argc = get_subj_from_db((subject_t**) &(rslt->arg));
            if(rslt->argc == -1)
            {
                rslt->argc = 0;
                rslt->rqst_rslt = obep_failure;
            }
            break;
        
        case obep_add_auth:
            rslt->rqst_rslt = (add_auth_to_db((author_t*) rqst->arg) ? obep_success : obep_failure);

            if(rslt->argc == -1)
            {
                rslt->argc = 0;
                rslt->rqst_rslt = obep_failure;
            }

            break;
        
        case obep_add_subj:
            rslt->rqst_rslt = (add_subj_to_db((subject_t*) rqst->arg) ? obep_success : obep_failure);
            
            if(rslt->argc == -1)
            {
                rslt->argc = 0;
                rslt->rqst_rslt = obep_failure;
            }

            break;
        
        case obep_add_book:
            rslt->rqst_rslt = (add_book_to_db((book_t*) rqst->arg) ? obep_success : obep_failure);

            if(rslt->argc == -1)
            {
                rslt->argc = 0;
                rslt->rqst_rslt = obep_failure;
            }
            break;
    };



    return rslt;
}



// MYSQL 