#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "obep_transport.h"
#include "sockets.h"

static int frame_size(const obep_msg_t* message, size_t* size)
{
    const char* header = message->header;
    if (header[4] != '|' || header[12] != '|' || header[15] != '|' ||
        header[OBEP__HEADER_LENGTH - 1] != '#')
    {
        errno = EPROTO;
        return -1;
    }
    for (size_t i = 0; i < 4; ++i)
    {
        if (!isxdigit((unsigned char) header[i]))
        {
            errno = EPROTO;
            return -1;
        }
    }

    char hex[5] = {0};
    memcpy(hex, header, 4);
    *size = strtoul(hex, NULL, 16);
    return 0;
}

int obep_send_frame(int socket, const obep_msg_t* message)
{
    size_t size;
    if (message == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    if (frame_size(message, &size) < 0)
        return -1;
    if (size > 0 && (message->msg == NULL || strlen(message->msg) + 1 != size))
    {
        errno = EINVAL;
        return -1;
    }
    if (cslib_send(socket, message->header, OBEP__HEADER_LENGTH) < 0 ||
        (size > 0 && cslib_send(socket, message->msg, size) < 0))
        return -1;
    return 0;
}

int obep_receive_frame(int socket, obep_msg_t* message)
{
    if (message == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    obep_msg_t received = {0};
    ssize_t count = cslib_receive(socket, received.header, OBEP__HEADER_LENGTH);
    if (count <= 0)
    {
        if (count == 0)
            errno = ECONNRESET;
        return -1;
    }

    size_t size;
    if (frame_size(&received, &size) < 0)
        return -1;
    if (size > 0)
    {
        received.msg = malloc(size);
        if (received.msg == NULL)
            return -1;
        count = cslib_receive(socket, received.msg, size);
        if (count <= 0 || memchr(received.msg, '\0', size) != received.msg + size - 1)
        {
            if (count >= 0)
                errno = count == 0 ? ECONNRESET : EPROTO;
            free(received.msg);
            return -1;
        }
    }
    *message = received;
    return 0;
}
