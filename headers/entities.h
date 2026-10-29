#ifndef __ENTITIES_H_
#define __ENTITIES_H_

#include <stddef.h>

#define auth_fname_max_len 20
#define auth_lname_max_len 20
#define auth_bdate_max_len 20
#define subj_name_max_len 20
#define book_isbn_max_len 20
#define book_title_max_len 100

typedef struct author_
{
    int id;
    char last_name[auth_lname_max_len];
    char first_name[auth_fname_max_len];
    char birth_date[auth_bdate_max_len];
} author_t;

typedef struct subject_
{
    int id;
    char name[subj_name_max_len];
} subject_t;

typedef struct book_
{
    int id;
    int processed;
    union 
    {
        author_t* auth;
        int auth_id;
    } auth;
    union
    {
        subject_t* subj;
        int subj_id;
    } subj;
    int page_count;
    int stock_quantity;
    float price;
    int publish_year;
    char title[book_title_max_len];
    char isbn[book_isbn_max_len];
} book_t;

typedef struct employee_
{
    int id;
    char* login;
    char* pswd;
} employee_t;

#endif