#include <stdio.h>
#include <string.h>

#include "crypto.h"
#include "host.h"
#include "operator.h"
#include "parser.h"

/**
 *  Exchange public keys with the server to encrypt all further communications.
 *  Send the server the public key, and keep reading the listening socket up to
 *  MAX_TRIES. If a packet comes in on the AUTH_CHANNEL, make sure it's the
 *  correct size and adopt it.
 *  @return: error code
 */
const char *operator_authenticate (void)
{
        u64 tries = 0;
        ENetEvent *ev = NULL;
        u8 authenticated = 0;
        const char *error = NULL;
        int len = 0;
        void *data = NULL;

        if (host_deliver(crypto_getkey(), CHANNEL_AUTH, NULL))
               error = "Failed to deliver packet.";

        while ((tries < MAX_TRIES) && !authenticated) {
                ev = host_listen(0);
                tries++;
                if (ev) {
                        len = crypto_length(ev->packet->dataLength);
                        data = crypto_make(ev->packet, NULL);
                        if (data) {
                                if (parser_auth(data, len) == 1)
                                        authenticated = 1;
                                free(data);
                        }
                }
        }

        if (!authenticated)
                error = "Failed to authenticate.";

        return error;
}

/**
 *  Identify with the server by choosing an alias. Send an encrypted alias
 *  packet and listen up to MAX_TRIES for a response containing the user's
 *  alias.
 *  @return: error code
 */
const char *operator_identify (const char *alias, const char *word)
{
        const char *error = NULL;

        if (host_deliver(parser_pack_alias(alias, word), CHANNEL_AUTH, NULL))
                error = "Failed to deliver packet.\n";

        return error;
}

/**
 *  Establish a tcp connection with the server.
 *  @address: server to contact
 *  @return: an error code
 */
const char *operator_connect (ENetAddress address)
{
        const char *error = NULL;

        if (host_init_client())
                error = "Failed to create host.";
        else if (connected)
                error = "You're already connected!";
        else {
                if (host_connect(&address))
                        error = "Connection attempt failed.";
        }

        return error;
}
