#include <stdio.h>
#include <stdlib.h>
#include "obep.h"
#include "entities.h"

int printrqst(void* arg);
int printmsg(void* arg);
int printauthor(void* arg);
int printsubject(void* arg);
int printbook(void* arg);
int printemployee(void* arg);

static const char* rqst_type_str(const obep_rqst_type_t rqst);
static const char* msg_type_str(const obep_msg_type_t msg);
static const char* rqst_rslt_str(const obep_rqst_rslt_t rslt);

#define xtostr(num) #num
#define tostr(num) xtostr(num)

#define AUTHORS_COUNT 4
author_t authors[] = {
    {1, "Tom", "Parker", "1/1/2030"},
    {2, "Peter", "Middleson", "23/11/1998"},
    {3, "Joane", "Of Arc", "smt in like 1400"},
    {4, "Marie", "Curie", "Too lazy to search"}
};

#define SUBJECTS_COUNT 7
subject_t subjects[] = {
    {1, "Sci-Fi"},
    {2, "Fantasy"},
    {3, "Thriller"},
    {4, "Real crimes"},
    {5, "SPAAAAACE"}, // RIP Tim Curry
    {6, "Romance"},
    {7, "Horror"}
};

#define BOOKS_COUNT 2
book_t books[] = {
    {1, 1, {&authors[2]}, {&subjects[5]}, 130, 3, 4.99f, 1450, "The bibble", "000-000-001 or smth"},
    {2, 0, {.auth_id = 4}, {.subj_id = 3}, 197, 5, 7.49f, 1920, "Radiation kills", "928387218393798237"}
};

int main(void)
{
    employee_t bob = {-1, "Bob", "Bob123"};

    #define rqsts_count 8
    obep_rqst_t send_rqsts[rqsts_count] = {
        {obep_login, obep_req, obep_norslt, 1, &bob},
        {obep_logout, obep_req, obep_norslt, 0, NULL},
        {obep_get_auth, obep_req, obep_norslt, 0, NULL},
        {obep_get_subj, obep_req, obep_norslt, 0, NULL},
        {obep_add_auth, obep_req, obep_norslt, 1, (void*) &authors[2]},
        {obep_add_subj, obep_req, obep_norslt, 1, (void*) &subjects[2]},
        {obep_add_book, obep_req, obep_norslt, 1, (void*) &books[0]},
        {obep_add_book, obep_req, obep_norslt, 1, (void*) &books[1]}
    };

    int (*printarg[rqsts_count]) (void* object) = {
        (int (*) (void*))printemployee, 
            NULL, 
            NULL,
            NULL,
            printauthor,
            printsubject,
            printbook,
            printbook
        };

    obep_msg_t* send_msgs[rqsts_count] = {0};

    for(int i = 0; i < rqsts_count; i++)
    {
        send_msgs[i] = obep_get_msg(&(send_rqsts[i]));
        printf("rqst %d: ", i);
        printrqst(&send_rqsts[i]);
        if(printarg[i] != NULL)
        {
            printf("\n\twith arg:   ");
            printarg[i](send_rqsts[i].arg);
        }

        printf("\n\n\tproduced msg: ");
        printmsg(send_msgs[i]);
        printf("\n\n\n");
    }

    for(int i = 0; i < rqsts_count; i++)
    {
        free(send_msgs[i]->msg);
        free(send_msgs[i]);
    }

    return 0;
}



int printrqst(void* arg)
{
    obep_rqst_t* rqst = (obep_rqst_t*) arg;
    return printf("REQUEST: rqst type: %s, msg type: %s, rslt type: %s, argc: %d", rqst_type_str(rqst->rqst_type), msg_type_str(rqst->msg_type), rqst_rslt_str(rqst->rqst_rslt), rqst->argc);
}

int printmsg(void* arg)
{
    obep_msg_t* msg = (obep_msg_t*) arg;
    return printf("MESSAGE: %." tostr(OBEP__HEADER_LENGTH) "s%s", msg->header, msg->msg);
}

int printauthor(void* arg)
{
    author_t* auth = (author_t*) arg;
    return printf("AUTHOR: ID: %d, lname: %s, fname: %s, bdate: %s", auth->id, auth->last_name, auth->first_name, auth->birth_date);
}

int printsubject(void* arg)
{
    subject_t* subj = (subject_t*) arg;
    return printf("SUBJECT: ID: %d, name: %s", subj->id, subj->name);
}

int printbook(void* arg)
{
    book_t* book = (book_t*) arg;
    return printf("BOOK: ID: %d, processed: %d, author_id: %d, subject_id: %d, pages: %d, stock: %d, price: %f, publish year: %d, title: %s, ISBN: %s",
                    book->id, book->processed, 
                    (book->processed ? book->auth.auth->id : book->auth.auth_id),
                    (book->processed ? book->subj.subj->id : book->subj.subj_id),
                    book->page_count, book->stock_quantity, book->price,
                    book->publish_year, book->title, book->isbn);
}

int printemployee(void* arg)
{
    employee_t* empl = (employee_t*) arg;
    return printf("EMPLOYEE: ID: %d, login: %s, password: %s", empl->id, empl->login, empl->pswd);
}

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