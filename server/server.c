#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <mysql.h>
#include "entities.h"
#include "sockets.h"
#include "obep.h"
#include "obep_transport.h"

#define CONFIG_FILE ".server.cfg"
#define PORT_ENCODING "PORT_ENCODING"
#define NTHREAD "NTHREAD"
#define BIND_ADDRESS "BIND_ADDRESS"
#define ADDRESS_SIZE 256
#define MYSQL_OPTIONS_FILE "./.mysql-dacsc.cnf"

#define loggedIn *(int*) pthread_getspecific(key_loggedIn)

#define UNUSED(x) (void)(x)

typedef struct node_
 {
    int fd;
    struct node_* next;
 } node_t;

int configure(char* filepath, int* port, int* nbthread, char address[ADDRESS_SIZE]);

void* threadHandler(void* arg);

void connectionHandler(int socket);
obep_rqst_t* processRequest(obep_rqst_t* rqst);

int check_employee_existence(const employee_t* empl);

int get_auth_from_db(author_t** array);
int get_subj_from_db(subject_t** array);

int add_auth_to_db(const author_t* auth);
int add_subj_to_db(const subject_t* subj);
int add_book_to_db(const book_t* book);

void waitlist_push(int fd);
int waitlist_pop(void);

pthread_key_t key_loggedIn;

pthread_mutex_t mysqlMutex = PTHREAD_MUTEX_INITIALIZER,
                waitlistMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t waitlistCond = PTHREAD_COND_INITIALIZER;

int  nbthread = 0;
pthread_t* threads;

MYSQL* mysqlConnection = NULL;

node_t* waitlist;

int main(void)
{
    if (pthread_key_create(&key_loggedIn, NULL) != 0)
    {
        fputs("Could not create worker thread keys\n", stderr);
        return EXIT_FAILURE;
    }

    int port = 0;
    char address[ADDRESS_SIZE] = "";

    if(configure(CONFIG_FILE, &port, &nbthread, address) == -1)
    {
        perror("Couldn't configure through config file");
        return EXIT_FAILURE;
    }

    mysqlConnection = mysql_init(NULL);
    if (mysqlConnection == NULL)
    {
        fputs("Could not initialize MySQL\n", stderr);
        return EXIT_FAILURE;
    }
    FILE* options = fopen(MYSQL_OPTIONS_FILE, "r");
    if (options == NULL)
    {
        perror(MYSQL_OPTIONS_FILE);
        mysql_close(mysqlConnection);
        return EXIT_FAILURE;
    }
    fclose(options);
    if (mysql_options(mysqlConnection, MYSQL_READ_DEFAULT_FILE, MYSQL_OPTIONS_FILE) != 0 ||
        mysql_options(mysqlConnection, MYSQL_READ_DEFAULT_GROUP, "dacsc") != 0 ||
        mysql_options(mysqlConnection, MYSQL_SET_CHARSET_NAME, "utf8mb4") != 0 ||
        mysql_real_connect(mysqlConnection, NULL, NULL, NULL, "PourStudent", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "MySQL connection failed: %s\n", mysql_error(mysqlConnection));
        mysql_close(mysqlConnection);
        return EXIT_FAILURE;
    }

    int lstnscket = cslib_serverSocketAt(address[0] ? address : NULL, port);
    if (lstnscket < 0)
    {
        fprintf(stderr, "%s:%d: %s\n", address[0] ? address : "*", port, cslib_socketerror_str(errno));
        mysql_close(mysqlConnection);
        return EXIT_FAILURE;
    }

    threads = calloc((size_t) nbthread, sizeof(*threads));
    if (threads == NULL)
        return EXIT_FAILURE;
    for (int i = 0; i < nbthread; i++)
    {
        if (pthread_create(&threads[i], NULL, threadHandler, NULL) != 0)
        {
            fputs("Could not create worker thread\n", stderr);
            return EXIT_FAILURE;
        }
    }
    printf("Listening on %s:%d (%d workers)\n", address[0] ? address : "*", port, nbthread);
    fflush(stdout);

    while(1)
    {
        int srvcscket = cslib_accept(lstnscket);

        if (srvcscket < 0)
        {
            if (errno == EINTR || errno == ECONNABORTED)
                continue;
            perror("accept");
            return EXIT_FAILURE;
        }

        pthread_mutex_lock(&waitlistMutex);

        waitlist_push(srvcscket);

        pthread_cond_signal(&waitlistCond);

        pthread_mutex_unlock(&waitlistMutex);
    }

    return 0;
}

int configure(char* filepath, int* port, int* nbthread, char address[ADDRESS_SIZE])
{
    FILE* config = fopen(filepath, "r");
    if (config == NULL)
        return -1;

    char line[512];
    while (fgets(line, sizeof(line), config) != NULL)
    {
        const char* start = line + strspn(line, " \t\r\n");
        if (!strchr(line, '\n') && !feof(config))
            goto invalid;
        if (*start == '\0' || *start == '#')
            continue;
        char key[32], value[ADDRESS_SIZE], extra;
        if (sscanf(line, " %31[^= :\t\r\n] %*1[=:] %255s %c", key, value, &extra) != 2)
            goto invalid;
        if (strcmp(key, BIND_ADDRESS) == 0 && address[0] == '\0')
        {
            strcpy(address, value);
            continue;
        }
        char* end;
        errno = 0;
        long number = strtol(value, &end, 10);
        if (errno != 0 || *end != '\0' || number < 1)
            goto invalid;
        if (strcmp(key, PORT_ENCODING) == 0 && *port == 0 && number <= 65535)
            *port = (int) number;
        else if (strcmp(key, NTHREAD) == 0 && *nbthread == 0 && number <= 128)
            *nbthread = (int) number;
        else
            goto invalid;
    }
    if (ferror(config) || *port == 0 || *nbthread == 0)
        goto invalid;
    return fclose(config);

invalid:
    fclose(config);
    errno = EINVAL;
    return -1;
}


void* threadHandler(void* arg)
{
    UNUSED(arg);
    int session = 0;
    if (pthread_setspecific(key_loggedIn, &session) != 0 || mysql_thread_init() != 0)
    {
        fputs("Could not initialize worker thread\n", stderr);
        exit(EXIT_FAILURE);
    }

    int socket = -1;

    while(1)
    {
        pthread_mutex_lock(&waitlistMutex);
        while((socket = waitlist_pop()) == -1) pthread_cond_wait(&waitlistCond, &waitlistMutex);
        pthread_mutex_unlock(&waitlistMutex);

        loggedIn = 0;
        connectionHandler(socket);
        close(socket);
    }
}

void connectionHandler(int socket)
{
    obep_msg_t msg;
    while (obep_receive_frame(socket, &msg) == 0)
    {
        obep_rqst_t* request = obep_get_rqst(&msg);
        free(msg.msg);
        if (request == NULL)
            break;

        if (request->msg_type != obep_req || request->rqst_rslt != obep_norslt ||
            request->rqst_type < obep_login || request->rqst_type > obep_add_book)
        {
            free(request->arg);
            free(request);
            break;
        }

        pthread_mutex_lock(&mysqlMutex);
        obep_rqst_t* result = processRequest(request);
        pthread_mutex_unlock(&mysqlMutex);
        free(request->arg);
        free(request);
        if (result == NULL)
            break;

        obep_msg_t* response = obep_get_msg(result);
        free(result->arg);
        free(result);
        if (response == NULL)
            break;
        int sent = obep_send_frame(socket, response);
        free(response->msg);
        free(response);
        if (sent < 0)
            break;
    }
}

obep_rqst_t* processRequest(obep_rqst_t* rqst)
{
    obep_rqst_t* rslt = calloc(1, sizeof(*rslt));
    if (rslt == NULL) return NULL;

    rslt->rqst_type = rqst->rqst_type;
    rslt->msg_type = obep_ans;

    if(!loggedIn && rslt->rqst_type != obep_login)
    {
        rslt->rqst_rslt = obep_failure;
        rslt->argc = 0;
        rslt->arg = NULL;
        return rslt;
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
            rslt->argc = 0;
            rslt->arg = NULL;
            rslt->rqst_rslt = obep_success;
            break;

        case obep_get_auth:
            rslt->argc = get_auth_from_db((author_t**) &(rslt->arg));
            if(rslt->argc == -1)
            {
                rslt->argc = 0;
                rslt->rqst_rslt = obep_failure;
            }
            else rslt->rqst_rslt = obep_success;
            break;

        case obep_get_subj:
            rslt->argc = get_subj_from_db((subject_t**) &(rslt->arg));
            if(rslt->argc == -1)
            {
                rslt->argc = 0;
                rslt->rqst_rslt = obep_failure;
            }
            else rslt->rqst_rslt = obep_success;
            break;
        
        case obep_add_auth:
            rslt->arg = malloc(sizeof(int));
            if(rslt->arg != NULL && (*(int*) rslt->arg = add_auth_to_db((author_t*) rqst->arg)) > 0)
            {
                rslt->argc = 1;
                rslt->rqst_rslt = obep_success;                
            }
            else
            {
                rslt->rqst_rslt = obep_failure;
                rslt->argc = 0;
                free(rslt->arg);
                rslt->arg = NULL;
            }

            break;
        
        case obep_add_subj:
            rslt->arg = malloc(sizeof(int));
            if(rslt->arg != NULL && (*(int*) rslt->arg = add_subj_to_db((subject_t*) rqst->arg)) > 0)
            {
                rslt->argc = 1;
                rslt->rqst_rslt = obep_success;                
            }
            else
            {
                rslt->rqst_rslt = obep_failure;
                rslt->argc = 0;
                free(rslt->arg);
                rslt->arg = NULL;
            }

            break;
        
        case obep_add_book:
            rslt->arg = malloc(sizeof(int));
            if(rslt->arg != NULL && (*(int*) rslt->arg = add_book_to_db((book_t*) rqst->arg)) > 0)
            {
                rslt->argc = 1;
                rslt->rqst_rslt = obep_success;                
            }
            else
            {
                rslt->rqst_rslt = obep_failure;
                rslt->argc = 0;
                free(rslt->arg);
                rslt->arg = NULL;
            }

            break;
    };

    return rslt;
}



// MYSQL 

/* The caller holds mysqlMutex throughout each lookup and insert. */
static int row_exists(const char* query)
{
    if (mysql_query(mysqlConnection, query) != 0)
        return -1;
    MYSQL_RES* result = mysql_store_result(mysqlConnection);
    if (result == NULL)
        return -1;
    const int found = mysql_num_rows(result) != 0;
    mysql_free_result(result);
    return found;
}

static int insert_id(const char* query)
{
    if (mysql_query(mysqlConnection, query) != 0)
        return -1;
    const my_ulonglong id = mysql_insert_id(mysqlConnection);
    return id > 0 && id <= INT_MAX ? (int) id : -1;
}

/* Hex literals keep UTF-8 values independent of the connector and SQL escape mode. */
#define SQL_TEXT "CONVERT(X'%s' USING utf8mb4)"

static void encode_value(char* output, const char* input)
{
    static const char digits[] = "0123456789ABCDEF";
    while (*input != '\0')
    {
        const unsigned char byte = (unsigned char) *input++;
        *output++ = digits[byte >> 4];
        *output++ = digits[byte & 15];
    }
    *output = '\0';
}

int check_employee_existence(const employee_t* empl)
{
    if (empl == NULL)
        return 0;
    char login[2 * employee_login_max_len + 1];
    char password[2 * employee_password_max_len + 1];
    encode_value(login, empl->login);
    encode_value(password, empl->pswd);
    char query[384];
    snprintf(query, sizeof(query),
             "SELECT id FROM employees WHERE login=" SQL_TEXT " AND CAST(password AS BINARY)=X'%s'",
             login, password);
    return row_exists(query) == 1;
}

int get_auth_from_db(author_t** auth)
{
    *auth = NULL;
    if(mysql_query(mysqlConnection, "SELECT id,last_name,first_name,birth_date FROM authors ORDER BY id;") != 0) return -1;

    MYSQL_RES* rslt;
    if((rslt = mysql_store_result(mysqlConnection)) == NULL) return -1;

    size_t rows = mysql_num_rows(rslt);

    if (rows == 0 || rows > INT_MAX)
    {
        mysql_free_result(rslt);
        return rows == 0 ? 0 : -1;
    }
    *auth = calloc(rows, sizeof(**auth));
    if (*auth == NULL)
    {
        mysql_free_result(rslt);
        return -1;
    }

    MYSQL_ROW row;
    for(int i = 0; (row = mysql_fetch_row(rslt)) != NULL; i++)
    {
        (*auth)[i].id = atoi(row[0]);
        strncpy((*auth)[i].last_name, row[1] ? row[1] : "", auth_lname_max_len);
        strncpy((*auth)[i].first_name, row[2] ? row[2] : "", auth_fname_max_len);
        strncpy((*auth)[i].birth_date, row[3] ? row[3] : "", auth_bdate_max_len);
    }

    mysql_free_result(rslt);
    return (int) rows;
}

int get_subj_from_db(subject_t** subj)
{

    *subj = NULL;
    if(mysql_query(mysqlConnection, "SELECT id,name FROM subjects ORDER BY id;") != 0) return -1;

    MYSQL_RES* rslt;
    if((rslt = mysql_store_result(mysqlConnection)) == NULL) return -1;

    size_t rows = mysql_num_rows(rslt);

    if (rows == 0 || rows > INT_MAX)
    {
        mysql_free_result(rslt);
        return rows == 0 ? 0 : -1;
    }
    *subj = calloc(rows, sizeof(**subj));
    if (*subj == NULL)
    {
        mysql_free_result(rslt);
        return -1;
    }

    MYSQL_ROW row;
    for(int i = 0; (row = mysql_fetch_row(rslt)) != NULL; i++)
    {
        (*subj)[i].id = atoi(row[0]);
        strncpy((*subj)[i].name, row[1] ? row[1] : "", subj_name_max_len);
    }

    mysql_free_result(rslt);
    return (int) rows;
}

int add_auth_to_db(const author_t* auth)
{
    char last[2 * auth_lname_max_len + 1];
    char first[2 * auth_fname_max_len + 1];
    char date[2 * auth_bdate_max_len + 1];
    encode_value(last, auth->last_name);
    encode_value(first, auth->first_name);
    encode_value(date, auth->birth_date);

    char query[384];
    snprintf(query, sizeof(query), "SELECT id FROM authors WHERE last_name=" SQL_TEXT " AND first_name=" SQL_TEXT, last, first);
    if (row_exists(query) != 0)
        return -1;
    snprintf(query, sizeof(query), "INSERT INTO authors (last_name,first_name,birth_date) VALUES (" SQL_TEXT "," SQL_TEXT "," SQL_TEXT ")",
             last, first, date);
    return insert_id(query);
}

int add_subj_to_db(const subject_t* subj)
{
    char name[2 * subj_name_max_len + 1];
    encode_value(name, subj->name);
    char query[128];
    snprintf(query, sizeof(query), "SELECT id FROM subjects WHERE name=" SQL_TEXT, name);
    if (row_exists(query) != 0)
        return -1;
    snprintf(query, sizeof(query), "INSERT INTO subjects (name) VALUES (" SQL_TEXT ")", name);
    return insert_id(query);
}

int add_book_to_db(const book_t* book)
{
    if (!isfinite(book->price))
        return -1;
    char title[2 * book_title_max_len + 1];
    char isbn[2 * book_isbn_max_len + 1];
    encode_value(title, book->title);
    encode_value(isbn, book->isbn);
    char query[768];
    snprintf(query, sizeof(query), "SELECT id FROM books WHERE isbn=" SQL_TEXT, isbn);
    if (row_exists(query) != 0)
        return -1;
    snprintf(query, sizeof(query),
             "INSERT INTO books (author_id,subject_id,page_count,stock_quantity,price,publish_year,title,isbn) "
             "VALUES (%d,%d,%d,%d,%.2f,%d," SQL_TEXT "," SQL_TEXT ")",
             book->processed ? book->auth.auth->id : book->auth.auth_id,
             book->processed ? book->subj.subj->id : book->subj.subj_id,
             book->page_count,
             book->stock_quantity, (double) book->price, book->publish_year, title, isbn);
    return insert_id(query);
}

// waitlist manipulation
void waitlist_push(int fd)
{
    node_t** indirect = &waitlist;

    while((*indirect) != NULL) indirect = &(*indirect)->next;

    *indirect = malloc(sizeof(node_t));
    if (*indirect == NULL)
    {
        close(fd);
        return;
    }

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
