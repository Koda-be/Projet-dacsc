#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entities.h"
#include "obep.h"

static const char* LOGIN_STR = "LOGIN  ";
static const char* LOGOUT_STR = "LOGOUT ";
static const char* GETAUTH_STR = "GETAUTH";
static const char* GETSUBJ_STR = "GETSUBJ";
static const char* ADDAUTH_STR = "ADDAUTH";
static const char* ADDSUBJ_STR = "ADDSUBJ";
static const char* ADDBOOK_STR = "ADDBOOK";

static const char* REQ_STR = "REQ";
static const char* ANS_STR = "ANS";

static const char* SUCC_STR = "SUCCESS";
static const char* FAIL_STR = "FAILURE";

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

#define SEPARATOR "|"

static char* req_get_arg_str(const obep_rqst_t* rqst);
static char* ans_get_arg_str(const obep_rqst_t* rqst);
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

    if(rqst->msg_type == obep_req) get_arg_str = req_get_arg_str;
    else get_arg_str = ans_get_arg_str;

    char* arg = get_arg_str(rqst);

    size_t argsize = strlen(arg);
    msg->msg = malloc(argsize+1);

    strncpy(msg->msg, arg, argsize);
    free(arg);
    
    sprintf(msg->header,    "%.4X|%.7s|%.3s", 
                            (int) argsize, 
                            rqst_type_str(rqst->rqst_type),
                            msg_type_str(rqst->msg_type));
    msg->header[OBEP__HEADER_LENGTH-1] = '#';

    return msg;
}



char* req_get_arg_str(const obep_rqst_t* rqst)
{
    switch(rqst->rqst_type)
    {
        case obep_add_auth: return get_add_auth_str((const author_t*) rqst->arg);
        case obep_add_subj: return get_add_subj_str((const subject_t*) rqst->arg);
        case obep_add_book: return get_add_book_str((const book_t*) rqst->arg);
        default: return NULL;
    }
}

char* get_add_book_str(const book_t* book)
{
    if(book->auth == NULL || book->subj == NULL) return NULL;

    // Holy hell that's one ugly format
    const char* format =    AUTH_ID ":%d" SEPARATOR
                            SUBJ_ID ":%d" SEPARATOR
                            PAGE_COUNT ":%d" SEPARATOR
                            STOCK ":%d" SEPARATOR
                            PRICE ":.2f" SEPARATOR
                            PUBLISH_YEAR ":%d" SEPARATOR
                            TITLE ":%." tostr(book_title_max_len) "s" SEPARATOR
                            ISBN ":%." tostr(book_isbn_max_len) "s";

    size_t maxstrlen = strlen(format)+100+20+60; // add 100 for title, 20 for isbn, 10 for each number

    char* str = malloc(maxstrlen);
    snprintf(str, maxstrlen, format,    
                            book->auth->id, 
                            book->subj->id,
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
        case obep_success: return SUCC_STR;
        case obep_failure: return FAIL_STR;
        default: return NULL;
    }
}