#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "obep_transport.h"
#include "sockets.h"

int obep_send_frame(int socket, const obep_msg_t* message)
{
    if (message == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    char longueurHex[5] = {0};
    memcpy(longueurHex, message->header, 4);

    char* fin;
    errno = 0;

    unsigned long longueur = strtoul(longueurHex, &fin, 16);

    if (errno != 0 || fin != longueurHex + 4 ||
        message->header[4] != '|' ||
        message->header[12] != '|' ||
        message->header[16] != '#' ||
        (longueur > 0 && message->msg == NULL))
    {
        errno = EINVAL;
        return -1;
    }

    if (cslib_sendAll(socket, message->header, OBEP__HEADER_LENGTH) < 0)
        return -1;

    if (longueur > 0)
    {
        if (cslib_sendAll(socket, message->msg, (size_t)longueur) < 0)
            return -1;
    }

    return 0;
}

int obep_receive_frame(int socket, obep_msg_t* message)
{
    if (message == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    obep_msg_t recu = {0};

    // 1. Recevoir les 17 octets de l'en-tete
    ssize_t n = cslib_receiveExact(
        socket,
        recu.header,
        OBEP__HEADER_LENGTH
    );

    if (n <= 0)
    {
        if (n == 0)
            errno = ECONNRESET;

        return -1;
    }

    // 2. Verifier les separateurs de l'en-tete
    if (recu.header[4] != '|' ||
        recu.header[12] != '|' ||
        recu.header[16] != '#')
    {
        errno = EPROTO;
        return -1;
    }

    // 3. Verifier les 4 caracteres hexadecimaux
    for (size_t i = 0; i < 4; i++)
    {
        char c = recu.header[i];

        if (!((c >= '0' && c <= '9') ||
              (c >= 'A' && c <= 'F') ||
              (c >= 'a' && c <= 'f')))
        {
            errno = EPROTO;
            return -1;
        }
    }

    // 4. Convertir la longueur hexadecimale
    char longueurHex[5];

    memcpy(longueurHex, recu.header, 4);
    longueurHex[4] = '\0';

    size_t longueur = (size_t)strtoul(longueurHex, NULL, 16);

    // 5. Allouer le buffer du contenu
    recu.msg = malloc(longueur + 1);

    if (recu.msg == NULL)
        return -1;

    // 6. Recevoir exactement le contenu annonce
    if (longueur > 0)
    {
        n = cslib_receiveExact(socket, recu.msg, longueur);

        if (n <= 0)
        {
            free(recu.msg);

            if (n == 0)
                errno = ECONNRESET;

            return -1;
        }
    }

    // 7. Terminer la chaine en memoire
    recu.msg[longueur] = '\0';

    // 8. Transmettre le message complet a l'appelant
    *message = recu;

    return 0;
}