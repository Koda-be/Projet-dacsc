#ifndef OBEP_TRANSPORT_H
#define OBEP_TRANSPORT_H

#include "obep.h"

int obep_send_frame(int socket, const obep_msg_t* message);
int obep_receive_frame(int socket, obep_msg_t* message);

#endif