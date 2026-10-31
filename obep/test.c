#include <stdio.h>
#include "obep.h"
#include "entities.h"

int printrqst(obep_rqst_t* rqst);
int printmsg(obep_msg_t* msg);
int printauthor(author_t* auth);
int printsubject(subject_t* subj);
int printbook(book_t* book);
int printemployee(employee_t* empl);

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
    {1, 1, {&authors[3]}, {&subjects[5]}, 130, 3, 4.99f, 1450, "The bibble", "000-000-001 or smth"},
    {2, 0, {.auth_id = 4}, {.subj_id = 3}, 197, 5, 7.49f, 1920, "Radiation kills", "928387218393798237"}
};

int main(void)
{
    

    return 0;
}



int printrqst(obep_rqst_t* rqst)
{
    return printf("REQUEST: rqst type: %s, msg type: %s, rslt type: %s, argc: %d", rqst_type_str(rqst->rqst_type), msg_type_str(rqst->msg_type), rqst_rslt_str(rqst->rqst_rslt), rqst->argc);
}

int printmsg(obep_msg_t* msg)
{
    return printf("MESSAGE: %." tostr(OBEP__HEADER_LENGTH) "s%s", msg->header, msg->msg);
}

int printauthor(author_t* auth)
{
    return printf("AUTHOR: ID: %d, lname: %s, fname: %s, bdate: %s", auth->id, auth->last_name, auth->first_name, auth->birth_date);
}

int printsubject(subject_t* subj)
{
    return printf("SUBJECT: ID: %d, name: %s", subj->id, subj->name);
}

int printbook(book_t* book)
{
    return printf("BOOK: ID: %d, processed: %d, author_id: %d, subject_id: %d, pages: %d, stock: %d, price: %f, publish year: %d, title: %s, ISBN: %s",
                    book->id, book->processed, 
                    (book->processed ? book->auth.auth->id : book->auth.auth_id),
                    (book->processed ? book->subj.subj->id : book->subj.subj_id),
                    book->page_count, book->stock_quantity, book->price,
                    book->publish_year, book->title, book->isbn);
}

int printemployee(employee_t* empl)
{
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