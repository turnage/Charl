#include <string.h>
#include <crypto.h>

#include "parser.h"
#include "operator.h"
#include "log.h"
#include "host.h"

/**
 *  Check a local line to see what kind of command it is, send it to the
 *  appropriate handler.
 *  @line: the currently activated string
 *  @return: 0 or an error for bad arguments
 */
int parser_out (const char *line)
{
        int report = 0;

        if (!strncmp(line, "/", 1)) {
                if (!strncmp(line, "/c", 2)) {
                        report = parser_out_connect(line);
                } else if (!strncmp(line, "/logi", 5) && connected) {
                        report = parser_out_login(line);
                } else if (!strncmp(line, "/hop", 4)) {
                        report = parser_out_hop(line);
                } else if (!strcmp(line, "/stop") && connected) {
                        report = parser_out_stop();
                } else if (!strcmp(line, "/disconnect")) {
                        host_disconnect();
                        log_add(COL_NOTIF, "Disconnected.\n");
                } else if (!strncmp(line, "/r", 2)) {
                        report = parser_out_register(line);
                } else if (!strncmp(line, "/mute", 5)) {
                        report = parser_out_mute(line);
                } else if (!strncmp(line, "/unmute", 7)) {
                        report = parser_out_unmute(line);
                } else if (!strncmp(line, "/logo", 5)) {
                        report = parser_out_logout();
                } else if (!strncmp(line, "/u", 2)) {
                        report = parser_out_unregister();
                } else {
                        log_add(COL_ERROR, "Unknown command.\n");
                }
        } else if (!connected) {
                        log_add(COL_ERROR, "You are not connected anywhere.\n");
        } else {
                host_deliver(parser_pack_talk(line), CHANNEL_PUBLIC, NULL);
                fprintf(stderr, "Delivered message.\n");
        }

        return report;
}

/**
 *  Listen for incoming packets that need attention. Redirect them to the
 *  necessary module.
 *  @return: value reserved for potential patches
 */
int parser_in ()
{
        int report = 0;
        
        ENetEvent *ev = host_listen(0);
        if (!ev)
                report = 1;
        else if (ev->type == ENET_EVENT_TYPE_DISCONNECT) {
                connected = 0;
                log_add(COL_NOTIF, "You were disconnected from the server.\n");
                char temp[MAX_BUF] = {0};
                temp[0] = ' ';
                log_list(temp);
        } else if (ev->type == ENET_EVENT_TYPE_RECEIVE) {
                int len = crypto_length(ev->packet->dataLength);
                char *data = crypto_make(ev->packet, NULL);
                if (data) {
                        if ((ev->channelID == CHANNEL_PUBLIC) &&
                            (len == sizeof(pack_talk))) {
                                char *msg = ((pack_talk *)data)->message;
                                log_add(COL_NONE, msg);
                        } else if ((ev->channelID == CHANNEL_NOTIF) &&
                                   (len == sizeof(pack_talk))) {
                                char *msg = ((pack_talk *)data)->message;
                                log_add(COL_NOTIF, msg);
                        } else if ((ev->channelID == CHANNEL_DATA) &&
                                   (len == sizeof(pack_talk))) {
                                char *msg = ((pack_talk *)data)->message;
                                log_list(msg);
                        }
                        free(data);
                }
        }

        return report;
}

/**
 *  Check to make sure a connect request is formatted in a valid way, and
 *  attempt to connect to the server.
 *  @line: the line to parse
 *  @return: 0 for success and an error for bad arguments
 */
int parser_out_connect (const char *line)
{
        int report = 0;
        ENetAddress address;
        char local[MAX_BUF];
        char *ip, *port;

        memset(local, 0, sizeof(local));
        memcpy(local, line, strlen(line));

        strtok(local, " ");

        if (!(ip = strtok(NULL, ":")) ||
            !(port = strtok(NULL, ":")))
                report = 1;
        else {
                if (enet_address_set_host(&address, ip))
                        report = 2;

                int portnum = atoi(port);
                if ((portnum < 1) || (portnum > 65535))
                        report = 3;
                else
                        address.port = portnum;
        }

        if (!report) {
                log_add(COL_NOTIF, "Connecting to %s:%s...\n", ip, port);
                const char *connect = operator_connect(address);
                if (connect)
                        log_add(COL_ERROR, "%s\n", connect);
                else {
                        log_add(COL_NOTIF, "Connected to server!\n");
                        const char *auth = operator_authenticate();
                        if (auth) {
                                log_add(COL_ERROR, auth);
                                host_disconnect();
                                log_add(COL_NOTIF, "Disconnected.\n");
                        }
                }
        } else if (report == 1)
                log_add(COL_ERROR, "Use the form 127.0.0.1:1235.\n");
        else if (report == 2)
                log_add(COL_ERROR, "Invalid address.\n");
        else if (report == 3)
                log_add(COL_ERROR, "Invalid port.\n");

        return report;
}

/**
 *  Check to make sure a login requst is formatted in a valid way, and attempt
 *  to claim an alias with the provided password.
 *  @line: the line to parse
 *  @return: 0 for success and an error for bad arguments
 */
int parser_out_login (const char *line)
{
        int report = 0;
        char local[MAX_BUF] = {0};
        char *name = NULL, *word = NULL;

        memcpy(local, line, strlen(line));

        strtok(local, " ");

        if (!(name = strtok(NULL, " "))) {
                report = 1;
                log_add(COL_ERROR, "You must provide a username.\n");
        } else {
                word = strtok(NULL, " ");
                const char *id = operator_identify(name, word);
                if (id)
                        log_add(COL_ERROR, id);
        }

        return report;
}

/**
 *  Check to make sure an attempt to hop channels is correctly formatted, and
 *  send the packet representing the request to the server.
 *  @line: the line to parse
 *  @return: report on the attempt
 */
int parser_out_hop (const char *line)
{
        int report = 0;
        char local[MAX_BUF] = {0};
        char *channel = NULL, *word = NULL;

        memcpy(local, line, strlen(line));
        strtok(local, " ");

        if (!(channel = strtok(NULL, " "))) {
                report = 1;
                log_add(COL_ERROR, "You must provide a channel.\n");
        } else {
                ENetPacket *pack = NULL;
                int id = atoi(channel);
                if (id < 0)
                        log_add(COL_ERROR, "Channels are positive integers.\n");
                else {
                        word = strtok(NULL, " ");
                        pack_chan temp;
                        memset(&temp, 0, sizeof(pack_chan));
                        if (word)
                                memcpy(temp.word, word, MAX_PASS);
                        temp.flag = PACK_CHAN;
                        temp.channel = id;
                        pack = crypto_encipher(&temp, sizeof(pack_chan), NULL);
                        report = host_deliver(pack, CHANNEL_DATA, NULL);
                }
        }

        return report;
}

/**
 *  Check a mute command for a username, and forward it to the server to handle.
 *  @line: the command to parse
 *  @return: a report on the attempt
 */
int parser_out_mute (const char *line)
{
        int report = 0;
        char local[MAX_BUF] = {0};
        char *name;

        memcpy(local, line, strlen(line));
        strtok(local, " ");

        if (!(name = strtok(NULL, " "))) {
                report = 1;
                log_add(COL_ERROR, "You must provide a username.\n");
        } else {
                int len = strlen(name);

                if ((len < MAX_NAME) && (len >= MIN_NAME)) {
                        ENetPacket *pack = NULL;
                        pack_mod temp;

                        memset(&temp, 0, sizeof(pack_mod));
                        temp.flag = PACK_MOD;
                        temp.operation = MOD_MUTE;
                        memcpy(temp.alias, name, strlen(name));

                        pack = crypto_encipher(&temp, sizeof(pack_mod), NULL);
                        report = host_deliver(pack, CHANNEL_MOD, NULL);
                }
        }

        return report;
}

/**
 *  Check an unmute command for a username, and forward it to the server to
 *  handle.
 *  @line: the command to parse
 *  @return: a report on the attempt
 */
int parser_out_unmute (const char *line)
{
        int report = 0;
        char local[MAX_BUF] = {0};
        char *name;

        memcpy(local, line, strlen(line));
        strtok(local, " ");

        if (!(name = strtok(NULL, " "))) {
                report = 1;
                log_add(COL_ERROR, "You must provide a username.\n");
        } else {
                int len = strlen(name);

                if ((len < MAX_NAME) && (len >= MIN_NAME)) {
                        ENetPacket *pack = NULL;
                        pack_mod temp;

                        memset(&temp, 0, sizeof(pack_mod));
                        temp.flag = PACK_MOD;
                        temp.operation = MOD_UNMUTE;
                        memcpy(temp.alias, name, strlen(name));

                        pack = crypto_encipher(&temp, sizeof(pack_mod), NULL);
                        report = host_deliver(pack, CHANNEL_MOD, NULL);
                }
        }

        return report;
}

/**
 *  Request that the server end the user session without disconnecting.
 *  @return: a report on the attempt.
 */
int parser_out_logout ()
{
        pack_alias temp;
        memset(&temp, 0, sizeof(pack_alias));
        temp.flag = PACK_LOGOUT;
        return host_deliver(crypto_encipher(&temp, sizeof(pack_alias), NULL),
                            CHANNEL_AUTH, NULL);
}

/**
 *  Request that the server delete the user profile without disconnecting or
 *  logging out.
 *  @return: a report on the attempt.
 */
int parser_out_unregister ()
{
        pack_alias temp;
        memset(&temp, 0, sizeof(pack_alias));
        temp.flag = PACK_UNREG;
        return host_deliver(crypto_encipher(&temp, sizeof(pack_alias), NULL),
                            CHANNEL_AUTH, NULL);
}

/**
 *  Verify that a registration attempt is correctly formatted and forward a
 *  packet for the server to process.
 *  @line: string which invoked the command
 *  @return: report on the attempt
 */
int parser_out_register (const char *line)
{
        int report = 0;
        char local[MAX_BUF] = {0};
        char *word = NULL;

        memcpy(local, line, strlen(line));

        strtok(local, " ");

        if (!(word = strtok(NULL, " "))) {
                report = 1;
                log_add(COL_ERROR, "You must provide a password.\n");
        } else {
                pack_alias temp;
                memset(&temp, 0, sizeof(pack_alias));
                temp.flag = PACK_ALIAS;

                sprintf(temp.word, "%s", word);

                report = host_deliver(crypto_encipher(&temp, sizeof(pack_alias),
                                      NULL), CHANNEL_AUTH, NULL);
        }

        return report;
}

/**
 *  Create a moderation request that would, when acted on, stop the server, and
 *  send it on.
 *  @return: report on delivering the packet
 */
int parser_out_stop ()
{
        pack_mod temp;

        memset(&temp, 0, sizeof(pack_mod));
        temp.flag = PACK_MOD;
        temp.operation = MOD_STOP;

        return host_deliver(crypto_encipher(&temp, sizeof(pack_mod), NULL),
                            CHANNEL_MOD, NULL);
}

/**
 *  Construct an identification packet using a name and a password.
 *  @alias: name
 *  @word: password
 *  @return: packet with the outlined contents
 */
ENetPacket *parser_pack_alias (const char *alias, const char *word)
{
        pack_alias temp;
        memset(&temp, 0, sizeof(pack_alias));

        temp.flag = PACK_ALIAS;
        memcpy(temp.alias, alias, strlen(alias));
        if (word)
                memcpy(temp.word, word, strlen(word));
        
        return crypto_encipher(&temp, sizeof(pack_alias), NULL);
}

/**
 *  Create a public chat packet and deliver to the server with the proper
 *  data structure.
 *  @msg: a MAX_BUF length string of the message
 */
ENetPacket *parser_pack_talk (const char *msg)
{
        pack_talk temp;
        memset(&temp, 0, sizeof(pack_talk));

        temp.flag = PACK_TALK;
        memcpy(temp.message, msg, strlen(msg));

        return crypto_encipher(&temp, sizeof(pack_talk), NULL);
}

/**
 *  Parse a packet received on the authentication channel.
 *  @data: packet data (decrypted)
 *  @len: length of that data
 *  @name: access to alias
 *  @return: code
 */
int parser_auth (void *data, int len)
{
        int report = 0;

        if (len == sizeof(pack_key)) {
                crypto_set_peer(((pack_key *)data)->pk, PEER_SERVER);
                report = 1;
        }

        return report;
}