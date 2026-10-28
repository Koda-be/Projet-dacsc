#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include "sockets.h"

#include "obep_transport.h"

int main(void)
{
    int sockets[2];

    // Creer deux sockets connectees
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1)
    {
        perror("socketpair");
        return 1;
    }

    // Construire une trame OBEP de test
    obep_msg_t envoye = {0};

    memcpy(envoye.header, "0005|LOGIN  |REQ#",
           OBEP__HEADER_LENGTH);

    envoye.msg = "hello";

    // Preparer la reception
    obep_msg_t recu = {0};

    int resultat = 1;

    if (obep_send_frame(sockets[0], &envoye) == -1)
    {
        perror("Envoi OBEP");
    }
    else if (obep_receive_frame(sockets[1], &recu) == -1)
    {
        perror("Reception OBEP");
    }
    else if (memcmp(envoye.header, recu.header,
                    OBEP__HEADER_LENGTH) != 0 ||
             strcmp(envoye.msg, recu.msg) != 0)
    {
        fprintf(stderr, "Les messages sont differents !\n");
    }
    else
    {
        printf("TEST REUSSI : trame OBEP transmise et recue !\n");
        resultat = 0;
    }

    free(recu.msg);

    // Test : reception sans donnees avec timeout
    if (cslib_setTimeouts(sockets[1], 2) == -1)
    {
        perror("Configuration timeout");
        resultat = 1;
    }
    else
    {
        obep_msg_t autreMessage = {0};

        errno = 0;

        int retour = obep_receive_frame(sockets[1], &autreMessage);

        if (retour == -1 &&
            (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            printf("TEST REUSSI : timeout detecte !\n");
        }
        else
        {
            fprintf(stderr, "ECHEC : timeout non detecte !\n");
            resultat = 1;
        }

        free(autreMessage.msg);
    }

    close(sockets[0]);
    close(sockets[1]);

    return resultat;
}