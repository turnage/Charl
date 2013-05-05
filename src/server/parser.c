#include <stdio.h>
#include <string.h>
#include <host.h>
#include <crypto.h>

#include "parser.h"
#include "client.h"

/**
 *  Load server attributes from file so the host can be created.
 *  @port: port to listen on
 *  @max: maximum amount of peers which the server can host
 *  @capup: upstream bandwidth throttle
 *  @capdown: downstream bandwidth throttle
 */
void parser_load_config (int *port, int *max, int *capup, int *capdown)
{
        char temp[MAX_BUF] = {0};
        FILE *file = fopen("profile/config.txt", "r");

        char *val = NULL;

        /* read the port */
        fgets(temp, sizeof(temp), file);
        strtok_r(temp, "=", &val);
        (*port) = atoi(val);

        /* read the room size */
        fgets(temp, sizeof(temp), file);
        strtok_r(temp, "=", &val);
        (*max) = atoi(val);

        /* read upstream bandwidth cap */
        fgets(temp, sizeof(temp), file);
        strtok_r(temp, "=", &val);
        (*capup) = atoi(val);

        /* read downstream bandwidth cap */
        fgets(temp, sizeof(temp), file);
        strtok_r(temp, "=", &val);
        (*capdown) = atoi(val);
}

/**
 *  See if the requested channel is locked by reading the lock file.
 *  @channel: channel to check for
 *  @word: pointer to an array of at least size MAX_PASS
 */
void parser_load_channels (int channel, char *word)
{
        FILE *file = fopen("profile/locks.txt", "r");
        char temp[MAX_BUF] = {0};

        char *flag = NULL;
        char *val = NULL;

        while (fgets(temp, sizeof(temp), file)) {

                flag = strtok_r(temp, "=", &val);

                if (atoi(flag) == channel)
                        memcpy(word, val, strlen(val));

        }
}

/**
 *  Parse and handle a packet received on the authentication channel, then relay
 *  confirmation to client(s).
 *  @data: packet data (decrypted or still plain)
 *  @len: length of that data
 *  @peer: person who sent the packet
 */
void parser_auth (const void *data, int len, ENetPeer *peer)
{
        if (len == sizeof(pack_key)) {
                parser_key((pack_key *)data, (client *)peer->data);
        } else if (len == sizeof(pack_alias)) {
                parser_alias((pack_alias *)data, (client *)peer->data);
        }
}

/**
 *  Make sure the client talking has the required priveleges and the message is
 *  of a valid size. If it is, forward the message to all the peers in the
 *  speaker's channel.
 *  @data: pack_talk data
 *  @len: length of that data
 *  @peer: the speaker
 */
void parser_public (const void *data, int len, ENetPeer *peer)
{
        client *cli = peer->data;
        const pack_talk *pack = data;
        pack_talk msg;

        memset(&msg, 0, sizeof(pack_talk));
        memcpy(&msg, pack, sizeof(pack_talk));

        if ((len == sizeof(pack_talk)) && (cli->state == STATE_ENABLED)) {
                fprintf(stderr, "Broadcasting public packet.\n");
                sprintf(msg.message, "%s: %s\n", cli->name, pack->message);
                host_broadcast(&msg, len, CHANNEL_PUBLIC, cli->channel);
        }
}

/**
 *  Verify that the user requesting the packet has adequate priviledge to do so,
 *  and execute if the command is suitable authorized.
 *  @data: pack_talk data
 *  @len: length of that data
 *  @peer: the speaker
 */
void parser_mod (const void *data, int len, ENetPeer *peer)
{
        const pack_mod *mod = data;
        client *cli = peer->data;
        if (mod->operation == MOD_STOP && cli->rank == RANK_ADMIN) {
                connected = 0;
        }
}

/**
 *  Take requests to change channel and forward data to peers.
 *  @data: pack_talk data
 *  @len: length of that data
 *  @peer: the speaker
 */
void parser_data (const void *data, int len, ENetPeer *peer)
{
        client *cli = peer->data;

        if (len == sizeof(pack_chan)) {
                const pack_chan *pack = data;
                char word[MAX_PASS] = {0};
                parser_load_channels(pack->channel, word);
                if (!word[0] || !memcmp(word, pack->word, MAX_PASS)) {
                        if (pack->channel > 0) {
                                int old = cli->channel;
                                cli->channel = pack->channel;
                                host_list(old);
                                host_list(cli->channel);
                                client_talk(cli, "You are now in channel %d.\n",
                                            cli->channel);
                        }
                } else
                        client_talk(cli, "Wrong password for that channel.\n");
        }
}

/**
 *  Take a client's key packet and keep the key.
 *  @pack: packet to extract the key from
 *  @cli: author of the packet
 */
void parser_key (pack_key *pack, client *cli)
{
        memcpy(cli->pk, pack->pk, crypto_box_PUBLICKEYBYTES);
        host_deliver(crypto_getkey(), CHANNEL_AUTH, cli->peer);
        host_list(cli->channel);
}

/**
 *  Handle a client's attempt to gain an alias, including registration.
 *  Registration packets will not carry an alias.
 *  @pack: packet to break down
 *  @cli: author of the packet
 */
void parser_alias (pack_alias *pack, client *cli)
{
        int len = strlen(pack->alias);
        if ((len >= MIN_NAME) && (len < MAX_NAME)) {
                int error = client_login(pack->alias, pack->word, cli);
                if(!error) {
                        fprintf(stderr, "%s logged in.\n", cli->name);
                        client_talk(cli, "You are now %s.\n", cli->name);
                        host_list(cli->channel);
                } else if (error == ERR_AUTH) {
                        fprintf(stderr, "Failed log in attempt.\n");
                        client_talk(cli, "Invalid username or password.\n");
                } else if (error == ERR_INVALID) {
                        fprintf(stderr, "Corrupted profile: %s\n", pack->alias);
                        client_talk(cli, "Your profile on the server is "
                                         "corrupt. Contact the admin.\n");
                } else {
                        fprintf(stderr, "Log in error: %d\n", error);
                }
        } else if (len >= MAX_NAME)
                client_talk(cli, "Your name exceeds the maximum length.\n");
        else if (cli->name[0] && cli->state && pack->word[0]) {
                int error = client_register(pack->word, cli);
                if (!error) {
                        fprintf(stderr, "%s registered.\n");
                        client_talk(cli, "You have locked your username.\n");
                } else if (error == ERR_AUTH) {
                        client_talk(cli, "You are already registered; "
                                         "unregister first.\n");
                }
        } else if (cli->name[0] && pack->flag == PACK_LOGOUT) {
                memset(cli->name, 0, MAX_NAME);
                cli->state = STATE_DISABLED;
                cli->rank = RANK_NONE;
                client_talk(cli, "You are now logged out.\n");
                host_list(cli->channel);
        } else if (cli->name[0] && pack->flag == PACK_UNREG) {
                if (client_registered(cli->name)) {
                        client_unregister(cli);
                        client_talk(cli, "You are unregistered.\n");
                } else
                        client_talk(cli, "You are not registered.\n");
        }
}

/**
 *  Test all the parts of the parser module.
 *  @return: a report on the test
 */
int parser_test ()
{
        int report = -2;

        char word[MAX_PASS] = {0};
        int port, max, capup, capdown;
        parser_load_config(&port, &max, &capup, &capdown);
        if (port > 0 && max > 0 && capup >= 0 && capdown >= 0)
                report += 1;

        parser_load_channels(-1, word);
        if (!strcmp(word, "keep_lock"))
                report += 1;

        return report;
}