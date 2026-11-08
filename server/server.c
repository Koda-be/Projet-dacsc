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

#define trace(format, ...) printf("[%d::%d] > " format "\n", getpid(), *(int*) pthread_getspecific(key_ID) __VA_OPT__(,) __VA_ARGS__);
#define loggedIn *(int*) pthread_getspecific(key_loggedIn)

typedef struct node_
 {
    int fd;
    struct node_* next;
 } node_t;

int configure(char* filepath, int* port, int* nbthread);

int* get_ID(void);

void* connectionHandler(void* arg);
obep_rqst_t* processRequest(obep_rqst_t* rqst);

int check_employee_existence(const employee_t* empl);

int get_auth_from_db(author_t** array);
int get_subj_from_db(subject_t** array);

int add_auth_to_db(const author_t* auth);
int add_subj_to_db(const subject_t* subj);
int add_book_to_db(const book_t* book);

void waitlist_push(int fd);
int waitlist_pop(void);

pthread_key_t   key_ID,
                key_loggedIn;

pthread_mutex_t mysqlMutex = PTHREAD_MUTEX_INITIALIZER,
                getIdMutex = PTHREAD_MUTEX_INITIALIZER,
                waitlistMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t waitlistCond = PTHREAD_COND_INITIALIZER;

int  nbthread = 0;
pthread_t threads;

MYSQL* mysqlConnection = NULL;

node_t* waitlist;
int waitlist_length;

int main(void)
{
    pthread_key_create(&key_ID, free);
    pthread_key_create(&key_loggedIn, free);

    pthread_setspecific(key_ID, (void*) get_ID());

    int port = 0;

    if(configure(CONFIG_FILE, &port, &nbthread) == -1) 
    {
        perror("Couldn't configure through config file");
        exit(0);
    }

    trace("port: %d, nbthread: %d", port, nbthread);

    mysqlConnection = mysql_init(NULL);
    if(mysql_real_connect(mysqlConnection, "localhost", "DACSC", "DACSC#CassLioB38@", "DACSC", 0,0,0) == NULL) 
    {
        trace("Error with mysql connection: %s", mysql_error(mysqlConnection));
        mysql_close(mysqlConnection);
        exit(0);
    }

    trace("Successfully connected to DB",);

    int lstnscket = cslib_serverSocket(port);

    waitlist = NULL;
    waitlist_length = 0;

    while(1)
    {
        trace("listening on socket %d...", lstnscket);
        int srvcscket = cslib_accept(lstnscket);

        trace("Accepted connection with socket %d", srvcscket);

        pthread_mutex_lock(&waitlistMutex);

        waitlist_push(srvcscket);

        pthread_cond_signal(&waitlistCond);

        pthread_mutex_unlock(&waitlistMutex);
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

int* get_ID(void)
{
    static int i = 0;
    
    pthread_mutex_lock(&getIdMutex);

    i++;

    int* next_ID = malloc(sizeof(int));

    *next_ID = i;

    pthread_mutex_unlock(&getIdMutex);

    return next_ID;
}

void* connectionHandler(void* arg)
{
    int socket = *(int*) arg;

    obep_msg_t msg;

    while(cslib_receive(socket, msg.header, OBEP__HEADER_LENGTH) != -1)
    {   
        trace("Received message %24s", msg.header); 

        msg.msg = malloc(obep_get_msg_size(&msg));

        obep_rqst_t* rqst = obep_get_rqst(&msg);
        free(msg.msg);

        pthread_mutex_lock(&mysqlMutex);

        obep_rqst_t* rslt = processRequest(rqst);

        if(rslt->rqst_rslt == obep_failure) trace("Rqst failed: mysql error: %s", mysql_error(mysqlConnection));

        pthread_mutex_unlock(&mysqlMutex);

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
            if(rslt->argc == -1)
            {
                rslt->argc = 0;
                rslt->rqst_rslt = obep_failure;
            }
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

#define MYSQL_RESERVED_CHAR "\"*;'"

int check_if_usable_str(const char* str);

int check_employee_existence(const employee_t* empl)
{
    if(!(check_if_usable_str(empl->login) && check_if_usable_str(empl->pswd))) return 0;

    const char format[] = "SELECT * FROM employees WHERE login=%20s AND password=%20s";
    char query[50 + employee_login_max_len + employee_password_max_len + 1];
    
    sprintf(query, format, empl->login, empl->pswd);

    if(mysql_query(mysqlConnection, query) != 0)
        return 0;

    MYSQL_RES* result;
    if((result = mysql_store_result(mysqlConnection)) == NULL || mysql_fetch_row(result) == NULL) return 0;

    return 1;
}

int get_auth_from_db(author_t** auth)
{
    *auth = NULL;
    if(mysql_query(mysqlConnection, "SELECT * FROM authors;") != 0) return -1;

    MYSQL_RES* rslt;
    if((rslt = mysql_store_result(mysqlConnection)) == NULL) return -1;

    size_t rows = mysql_num_rows(rslt);

    *auth = (author_t*) malloc(rows*sizeof(author_t));

    MYSQL_ROW row;
    for(int i = 0; (row = mysql_fetch_row(rslt)) != NULL; i++)
    {
        (*auth)[i].id = atoi(row[0]);
        strncpy((*auth)[i].last_name, row[1], auth_lname_max_len);
        strncpy((*auth)[i].first_name, row[2], auth_lname_max_len);
        strncpy((*auth)[i].birth_date, row[3], auth_bdate_max_len);
    }

    return (int) rows;
}

int get_subj_from_db(subject_t** subj)
{

    *subj = NULL;
    if(mysql_query(mysqlConnection, "SELECT * FROM subjects;") != 0) return -1;

    MYSQL_RES* rslt;
    if((rslt = mysql_store_result(mysqlConnection)) == NULL) return -1;

    size_t rows = mysql_num_rows(rslt);

    *subj = (subject_t*) malloc(rows*sizeof(subject_t));

    MYSQL_ROW row;
    for(int i = 0; (row = mysql_fetch_row(rslt)) != NULL; i++)
    {
        (*subj)[i].id = atoi(row[0]);
        strncpy((*subj)[i].name, row[1], subj_name_max_len);
    }

    return (int) rows;
}

int add_auth_to_db(const author_t* auth)
{
    if(!(check_if_usable_str(auth->last_name) && check_if_usable_str(auth->first_name) && check_if_usable_str(auth->birth_date))) return -1;

    const char* selectFormat = "SELECT * FROM authors WHERE last_name=\"%20s\" AND first_name=\"%20s\";";
    char selectQuery[68+auth_lname_max_len + auth_fname_max_len + 1];

    sprintf(selectQuery, selectFormat, auth->last_name, auth->first_name);

    if(mysql_query(mysqlConnection, selectQuery) != 0) return -1;

    MYSQL_RES* rslt;
    if((rslt = mysql_store_result(mysqlConnection)) == NULL) return -1;

    if(mysql_num_rows(rslt) != 0) return -1;


    const char* insertFormat = "INSERT INTO authors (last_name, first_name, birth_date) values(%20s, %20s, %20s);";
    char insertQuery[82 + auth_lname_max_len + auth_fname_max_len + auth_bdate_max_len + 1];

    sprintf(insertQuery, insertFormat, auth->last_name, auth->first_name, auth->birth_date);

    if(mysql_query(mysqlConnection, insertQuery) != 0) return -1;

    if((rslt = mysql_store_result(mysqlConnection)) == NULL) return -1;

    MYSQL_ROW row = mysql_fetch_row(rslt);
    return atoi(row[0]);
}

int add_subj_to_db(const subject_t* subj)
{
    if(!check_if_usable_str(subj->name)) return -1;

    const char* selectFormat = "SELECT * FROM subjects WHERE name=\"%20s\";";
    char selectQuery[42+ subj_name_max_len + 1];

    sprintf(selectQuery, selectFormat, subj->name);

    if(mysql_query(mysqlConnection, selectQuery) != 0) return -1;

    MYSQL_RES* rslt;
    if((rslt = mysql_store_result(mysqlConnection)) == NULL) return -1;

    if(mysql_num_rows(rslt) != 0) return -1;

    const char* format = "INSERT INTO subjects (name) values(%20s);";
    char  query[42 + subj_name_max_len + 1];

    sprintf(query, format, subj->name);

    if(mysql_query(mysqlConnection, query) != 0) return -1;

    if((rslt = mysql_store_result(mysqlConnection)) == NULL) return -1;

    MYSQL_ROW row = mysql_fetch_row(rslt);
    return atoi(row[0]);
}

int add_book_to_db(const book_t* book)
{
    if(!(check_if_usable_str(book->isbn) && check_if_usable_str(book->title))) return -1;

    const char* format = "INSERT INTO books (author_id, subject_id, page_count, stock_quantity, price, publish_year, title, isbn) values(%d, %d, %d, %d, %f, %d, %100s, %20s);";
    char  query[149 + book_title_max_len + book_isbn_max_len + 2 + 11 * 6 + 1];

    sprintf(query,      format, 
                        (book->processed ? book->auth.auth->id : book->auth.auth_id), 
                        (book->processed ? book->subj.subj->id : book->subj.subj_id),
                    book->page_count, book->stock_quantity, book->price, book->publish_year, book->title, book->isbn);

    if(mysql_query(mysqlConnection, query) != 0) return -1;

    return 0;
}

int check_if_usable_str(const char* str)
{
    size_t strlength = strlen(str);
    int rsrvdlenght = strlen(MYSQL_RESERVED_CHAR);

    for(size_t i = 0; i <= strlength; i++)
        for(int j = 0; j <= rsrvdlenght; j++)
            if(str[i] == MYSQL_RESERVED_CHAR[j]) return 0;
    
    return 1;
}

// waitlist manipulation
void waitlist_push(int fd)
{
    node_t** indirect = &waitlist;

    while((*indirect) != NULL) indirect = &(*indirect)->next;

    *indirect = malloc(sizeof(node_t));

    (*indirect)->next = NULL;

    (*indirect)->fd = fd;
}

int waitlist_pop(void)
{
    node_t* first = waitlist;

    if(first == NULL) return -1;

    waitlist = first->next;

    int result = first->fd;
    free(first);

    return result;
}