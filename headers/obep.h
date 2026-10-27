#ifndef __OBEP_H_
#define __OBEP_H_

#include "entities.h"
#include <stddef.h>

#define OBEP__HEADER_LENGTH 17

typedef enum
{
    obep_login,
    obep_logout,
    obep_get_auth,
    obep_get_subj,
    obep_add_auth,
    obep_add_subj,
    obep_add_book,
} obep_rqst_type_t;

typedef enum
{
    obep_req,
    obep_ans
} obep_msg_type_t;

typedef enum
{
    obep_success,
    obep_failure
} obep_rqst_rslt_t;

// This is the raw message that will be sent and read from the input stream
typedef struct obep_msg_
{
    char header[OBEP__HEADER_LENGTH];
    char* msg;
} obep_msg_t;

// this is the processed request that will contain all the info needed 
typedef struct obep_rqst_
{
    obep_rqst_type_t rqst_type;
    obep_msg_type_t msg_type;
    void* arg;
} obep_rqst_t;

obep_msg_t* obep_get_msg(const obep_rqst_t* rqst);
obep_rqst_t* obep_get_rqst(const obep_msg_t* msg);

size_t obep_get_msg_size(obep_msg_t* msg);

#endif