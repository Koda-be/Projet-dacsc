#ifndef __ENTITIES_H_
#define __ENTITIES_H_

typedef struct author_
{
    int id;
    char* lname;
    char* fname;
    char* bdate;
} author_t;

typedef struct subject_
{
    int id;
    char* name;
} subject_t;

typedef struct book_
{
    int id;
    author_t* auth;
    subject_t* subj;
    char* title;
    char* isbn;
    int page_count;
    int stock_quantity;
    float price;
    int publish_year;
} book_t;

typedef struct employee_
{
    int id;
    char* login;
    char* pswd;
} employee_t;

#endif