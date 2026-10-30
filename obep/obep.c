#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entities.h"
#include "obep.h"

static const char* LOGIN_STR =      "LOGIN  ";
static const char* LOGOUT_STR =     "LOGOUT ";
static const char* GETAUTH_STR =    "GETAUTH";
static const char* GETSUBJ_STR =    "GETSUBJ";
static const char* ADDAUTH_STR =    "ADDAUTH";
static const char* ADDSUBJ_STR =    "ADDSUBJ";
static const char* ADDBOOK_STR =    "ADDBOOK";

static const char* REQ_STR = "RE";
static const char* ANS_STR = "AN";

static const char* NORSLT_STR = "       ";
static const char* SUCC_STR =   "SUCCESS";
static const char* FAIL_STR =   "FAILURE";

// Employee field strings
#define LOGIN "login"
#define PASSWORD "password:"

// Author field strings
#define LNAME "lastname:"
#define FNAME "firstname:"
#define BDATE "birthdate:"

// Subject field strings
#define NAME "name"

// Book field strings
#define AUTH_ID "auth_id"
#define SUBJ_ID "subj_id"
#define PAGE_COUNT "pagecount"
#define STOCK "stock"
#define PRICE "price"
#define PUBLISH_YEAR "publishyear"
#define TITLE "title"
#define ISBN "isbn"

#define xtostr(num) #num
#define tostr(num) xtostr(num)

#define FIELD_SEPARATOR "|"
#define ARG_SEPARATOR "@"

static char* req_get_arg_str(const obep_rqst_t* rqst);
static char* ans_get_arg_str(const obep_rqst_t* rqst);
static char* get_login_str(const employee_t* emp);
static char* get_get_auth_str(const obep_rqst_t* rqst);
static char* get_get_subj_str(const obep_rqst_t* rqst);
static char* get_add_auth_str(const author_t* author);
static char* get_add_subj_str(const subject_t* subject);
static char* get_add_book_str(const book_t* book);

static const char* rqst_type_str(const obep_rqst_type_t rqst);
static const char* msg_type_str(const obep_msg_type_t msg);
static const char* rqst_rslt_str(const obep_rqst_rslt_t rslt);


obep_msg_t* obep_get_msg(const obep_rqst_t* rqst)
{
    obep_msg_t* msg = (obep_msg_t*) malloc(sizeof(obep_msg_t));

    char* (* get_arg_str) (const obep_rqst_t* rqst);

    get_arg_str = ((rqst->msg_type == obep_req )? req_get_arg_str : ans_get_arg_str);

    char* arg = get_arg_str(rqst);

    size_t argsize = strlen(arg);
    msg->msg = malloc(argsize+1);

    strncpy(msg->msg, arg, argsize);
    free(arg);
    
    sprintf(msg->header,    "%.4X|%.7s|%.2s|%.7s", 
                            (int) argsize, 
                            rqst_type_str(rqst->rqst_type),
                            msg_type_str(rqst->msg_type),
                            rqst_rslt_str(rqst->rqst_rslt));
    msg->header[OBEP__HEADER_LENGTH-1] = '#';

    return msg;
}



char* req_get_arg_str(const obep_rqst_t* rqst)
{
    switch(rqst->rqst_type)
    {
        case obep_login:    return get_login_str((const employee_t*) rqst->arg);
        case obep_add_auth: return get_add_auth_str((const author_t*) rqst->arg);
        case obep_add_subj: return get_add_subj_str((const subject_t*) rqst->arg);
        case obep_add_book: return get_add_book_str((const book_t*) rqst->arg);
        default: return NULL;
    }
}

char* ans_get_arg_str(const obep_rqst_t* rqst)
{
    switch(rqst->rqst_type)
    {
        case obep_get_auth: return get_get_auth_str(rqst);
        case obep_get_subj: return get_get_subj_str(rqst);
        default: return NULL;
    }
}

char* get_login_str(const employee_t* empl)
{
    if (empl == NULL) return NULL;
    const char* format =    LOGIN ":%." tostr(employee_login_max_len) "s" FIELD_SEPARATOR
                            PASSWORD ":%." tostr(employee_password_max_len) "s";

    size_t maxstrlen = strlen(format) + employee_login_max_len + employee_password_max_len;

    char* str = (char*) malloc(maxstrlen + 1);
    snprintf(str, maxstrlen + 1, format, empl->login, empl->pswd);

    return str;
}

char* get_get_auth_str(const obep_rqst_t* rqst)
{
    if (rqst == NULL || rqst->arg == NULL || rqst->argc == 0) return NULL;
    
    const char* format =    LNAME ":%." tostr(auth_lname_max_len) "s" FIELD_SEPARATOR
                            FNAME ":%." tostr(auth_fname_max_len) "s" FIELD_SEPARATOR
                            BDATE ":%." tostr(auth_bdate_max_len) "s" ARG_SEPARATOR;

    size_t maxstr1len = strlen(format) + auth_lname_max_len + auth_fname_max_len + auth_bdate_max_len;
    char* str1 = (char*) malloc(maxstr1len+1);

    size_t maxstrlen = (size_t) rqst->argc * maxstr1len;
    char* str = (char*) malloc(maxstrlen+1); 

    const author_t* authors = (author_t*) rqst->arg;

    for(int i = 0; i < rqst->argc; i++)
    {
        snprintf(str1, maxstr1len, format, authors[i].last_name, authors[i].first_name, authors[i].birth_date);
        strncat(str, str1, maxstr1len);
    }

    free(str1);

    return str;
}

char* get_get_subj_str(const obep_rqst_t* rqst)
{
    if (rqst == NULL || rqst->arg == NULL || rqst->argc == 0) return NULL;
    
    const char* format =    NAME ":%." tostr(subj_name_max_len) "s" ARG_SEPARATOR;

    size_t maxstr1len = strlen(format) + subj_name_max_len;
    char* str1 = (char*) malloc(maxstr1len + 1);

    size_t maxstrlen = (size_t) rqst->argc * maxstr1len;
    char* str = (char*) malloc(maxstrlen+1);

    const subject_t* subjects = (subject_t*) rqst->arg;

    for(int i = 0; i < rqst->argc; i++)
    {
        snprintf(str1, maxstr1len, format, subjects[i].name);
        strncat(str, str1, maxstr1len);
    }

    free(str1);

    return str;
}

char* get_add_auth_str(const author_t* auth)
{
    if (auth == NULL) return NULL;

    const char* format =    LNAME ":%." tostr(auth_lname_max_len) "s" FIELD_SEPARATOR
                            FNAME ":%." tostr(auth_fname_max_len) "s" FIELD_SEPARATOR
                            BDATE ":%." tostr(auth_bdate_max_len) "s";

    size_t maxstrlen = strlen(format) + auth_lname_max_len + auth_fname_max_len + auth_bdate_max_len;

    char* str = (char*) malloc(maxstrlen+1);

    snprintf(str, maxstrlen + 1, format, auth->last_name, auth->first_name, auth->birth_date);

    return str;
}

char* get_add_subj_str(const subject_t* subj)
{
    if(subj == NULL) return NULL;

    const char* format =    NAME ":%." tostr(subj_name_max_len) "s";

    size_t maxstrlen = strlen(format) + subj_name_max_len;
    char* str = (char*) malloc(maxstrlen+1);

    snprintf(str, maxstrlen + 1, format, subj->name);

    return str;
}

char* get_add_book_str(const book_t* book)
{
    if(book == NULL) return NULL;

    if((book->processed && (book->auth.auth == NULL || book->subj.subj == NULL)) || book->auth.auth_id == 0 || book->subj.subj_id == 0) return NULL; 
    
    // Holy hell that's one ugly format
    const char* format =    AUTH_ID ":%d" FIELD_SEPARATOR
                            SUBJ_ID ":%d" FIELD_SEPARATOR
                            PAGE_COUNT ":%d" FIELD_SEPARATOR
                            STOCK ":%d" FIELD_SEPARATOR
                            PRICE ":%.2f" FIELD_SEPARATOR
                            PUBLISH_YEAR ":%d" FIELD_SEPARATOR
                            TITLE ":%." tostr(book_title_max_len) "s" FIELD_SEPARATOR
                            ISBN ":%." tostr(book_isbn_max_len) "s";

    size_t maxstrlen = strlen(format)+book_title_max_len+book_isbn_max_len+60; // add 100 for title, 20 for isbn, 10 for each number

    char* str = (char*) malloc(maxstrlen+1);
    snprintf(str, maxstrlen + 1, format,    
                            (book->processed ? book->auth.auth->id : book->auth.auth_id), 
                            (book->processed ? book->subj.subj->id : book->subj.subj_id),
                            book->page_count,
                            book->stock_quantity,
                            book->price,
                            book->publish_year,
                            book->title,
                            book->isbn);

    return str;
}

const char* rqst_type_str(const obep_rqst_type_t rqst)
{
    switch(rqst)
    {
        case obep_login: return LOGIN_STR;
        case obep_logout: return LOGOUT_STR;
        case obep_get_auth: return GETAUTH_STR;
        case obep_get_subj: return GETSUBJ_STR;
        case obep_add_auth: return ADDAUTH_STR;
        case obep_add_subj: return ADDSUBJ_STR;
        case obep_add_book: return ADDBOOK_STR;
        default: return NULL;
    }
}

const char* msg_type_str(const obep_msg_type_t msg)
{
    switch(msg)
    {
        case obep_ans: return ANS_STR;
        case obep_req: return REQ_STR;
        default: return NULL;
    }
}

const char* rqst_rslt_str(const obep_rqst_rslt_t rslt)
{
    switch(rslt)
    {
        case obep_norslt:   return NORSLT_STR;
        case obep_success:  return SUCC_STR;
        case obep_failure:  return FAIL_STR;
        default: return NULL;
    }
}