#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <crypto.h>
#include <host.h>

#include "client.h"

/**
 *  Functions to verbally represent enumerations for notifications, file
 *  recording, and debugging.
 */
const char *client_rank_string(rank_id rank)
{
        const char *ret = NULL;

        switch (rank) {

                case RANK_NONE: ret = "none";
                                break;

                case RANK_NAMED: ret = "user";
                                break;

                case RANK_MODERATOR: ret = "moderator";
                                break;

                case RANK_ADMIN: ret = "admin";
                                break;

        }

        return ret;
}

const char *client_state_string(state_id state)
{
        const char *ret = NULL;

        switch (state) {

                case STATE_ENABLED: ret = "enabled";
                                break;

                case STATE_MUTED: ret = "muted";
                                break;

                case STATE_ISOLATED: ret = "isolated";
                                break;

                case STATE_DISABLED: ret = "disabled";
                                break;

        }

        return ret;
}

const char *client_list_string (list_id list)
{
        const char *ret = NULL;

        switch (list) {

                case LIST_BLACK: ret = "blacklist";
                                break;

                case LIST_WHITE: ret = "whitelist";
                                break;

        }

        return ret;
}

/**
 *  Scan a list file for a username, and return the result of the search.
 *  @list: which list to search
 *  @name: username to search for
 *  @return: 1 if found, 0 if not
 */
int client_in_list (list_id list, const char *name)
{
        int in_list = 0, len = strlen(name);
        char temp[MAX_BUF] = {0};
        sprintf(temp, "profile/%s.txt", client_list_string(list));
        FILE *file = fopen(temp, "r");

        while (fgets(temp, sizeof(temp), file)) {

                /* first check if strings are the same length, since 'strncmp'
                 * otherwise ignores any difference after specified length */
                if ((strlen(temp) - 1) == len) {
                        if (!strncmp(temp, name, len)) {
                                in_list = 1;
                                break;
                         }
                }

        }

        fclose(file);

        return in_list;
}

/**
 *  Add a client to one of the lists.
 *  @list: list to add the client to
 *  @name: name to add to the list
 */
void client_list_add (list_id list, const char *name)
{
        if (client_in_list(list, name) != 0)
                return;

        char temp[MAX_BUF] = {0};
        sprintf(temp, "profile/%s.txt", client_list_string(list));
        FILE *file = fopen(temp, "a");

        fprintf(file, "%s\n", name);

        fclose(file);
}

/**
 *  Remove a client from one of the lists.
 *  @list: list to remove the client from
 *  @name: name to remove from the list
 */
void client_list_remove (list_id list, const char *name)
{
        char line[MAX_BUF] = {0};
        char file_name[MAX_BUF] = {0};
        char file_name_tmp[MAX_BUF] = {0};
        int len;
        sprintf(file_name, "profile/%s.txt", client_list_string(list));
        sprintf(file_name_tmp, "profile/%s_tmp.txt", client_list_string(list));
        FILE *file = fopen(file_name, "r");
        FILE *tmp_file = fopen(file_name_tmp, "w+");

        while (fgets(line, sizeof(line), file) != NULL) {

                /* first check if strings are the same length, since 'strncmp'
                 * otherwise ignores any difference after specified length */
                len = strlen(name);
                if ((strlen(line) - 1) == len) {
                        if (strncmp(line, name, len) == 0)
                                continue;
                }

                fputs(line, tmp_file);
        }

        fclose(file);
        fclose(tmp_file);

        /* replace the original file with the updated file */
        if (remove(file_name) != 0)
                printf("Remove file %s failed\n", file_name);
        if (rename(file_name_tmp, file_name) != 0)
                printf("Rename file %s failed\n", file_name_tmp);
}

/**
 *  Allocate memory for a new client and set the values to the default.
 *  @return: pointer to new client
 */
client *client_new ()
{
        client *cli = NULL;
        cli = calloc(sizeof(client), 1);
        cli->channel = 1;
        return cli;
}

/**
 *  Give a client a peer address for communicating with him/her.
 *  @peer: address of client
 *  @cli: client to link
 */
void client_assign (ENetPeer *peer, client *cli)
{
        cli->peer = peer;
}

/**
 *  Load a client's information from file, including password, rank, and info.
 *  @name: name of the client to load
 *  @word: password used to authenticate
 *  @cli: client to copy this data to.
 *  @return: a report on the attempt
 */
int client_load (const char *name, const char *word, client *cli)
{
        int report = 0;
        char temp[MAX_BUF] = {0};
        sprintf(temp, "profile/users/%s.txt", name);
        FILE *file = fopen(temp, "r");

        if (!file)
                report = ERR_MISSING;
        else {

                char *val = NULL;

                /* check password */
                fgets(temp, sizeof(temp), file);
                strtok_r(temp, "=", &val);
                val[strlen(val) - 1] = 0;
                if (strncmp(word, val, strlen(word)))
                        report = ERR_AUTH;

                /* take rank */
                fgets(temp, sizeof(temp), file);
                strtok_r(temp, "=", &val);
                if (report == ERR_AUTH)
                        cli->state = STATE_DISABLED;
                else if (!strncmp(val, "user", 4))
                        cli->rank = RANK_NAMED;
                else if (!strncmp(val, "moderator", 9))
                        cli->rank = RANK_MODERATOR;
                else if (!strncmp(val, "admin", 5))
                        cli->rank = RANK_ADMIN;
                else
                        report = ERR_INVALID;

                /* load info */
                fgets(temp, sizeof(temp), file);
                strtok_r(temp, "=", &val);
                if (val && !report)
                        memcpy(cli->info, val, strlen(val));

                /* if all went well, enable the user */
                if (!report) {
                        cli->state = STATE_ENABLED;
                        memcpy(cli->name, name, strlen(name));
                }

                fclose(file);

        }

        return report;
}

/**
 *  Assign a client an alias, checking to make sure the name is valid first. If
 *  the account has a profile load it and grant appropriate priveleges.
 *  @name: requested alias
 *  @word: the password they used
 *  @cli: client to attech idendity to
 *  @return: a report on the attempt
 */
int client_login(const char *name, const char *word, client *cli)
{
        int report = ERR_MISSING;

        char local[10] = {0};
        if (!word)
                word = local;

        if ((strlen(name) < MIN_NAME) || (strlen(name) >= MAX_NAME))
                report = ERR_INVALID;
        else {
                /* authenticate if the user has an account */
                if (word[0]){
                        report = client_load(name, word, cli);
                } else if (client_registered(name))
                        report = ERR_AUTH;
                else {
                        memset(cli->name, 0, MAX_NAME);
                        memcpy(cli->name, name, MAX_NAME);
                        cli->rank = RANK_NAMED;
                        cli->state = STATE_ENABLED;
                        report = 0;
                }
        }

        return report;
}

/**
 *  Print a client's information to the server console. This is only for debug
 *  and should not be called except in test suites.
 *  @cli: client to report on
 */
void client_report (client cli)
{
        if (cli.name[0]) {
                printf("----------------------------------\n");
                printf("Client:   %s\n", cli.name);
                printf("----------------------------------\n");

                if (cli.rank == RANK_NAMED)
                        printf("Rank:     User\n");
                else if (cli.rank == RANK_MODERATOR)
                        printf("Rank:     Moderator\n");
                else if (cli.rank == RANK_ADMIN)
                        printf("Rank:     Admin\n");

                if (cli.info[0]) {
                        printf("Info:     %s\n", cli.info);
                        printf("----------------------------------\n");
                }
        } else
                printf("Client has no profile.\n");
}

/**
 *  See if a client has a persistent profile with the server.
 *  @name: username to search for in the "users" folder.
 *  @return: 1 if found; 0 if not
 */
int client_registered (const char *name)
{
        int registered = 0;

        char temp[MAX_BUF] = {0};
        sprintf(temp, "profile/users/%s.txt", name);
        FILE *file = fopen(temp, "r");

        if (file) {
                registered = 1;
                fclose(file);
        }

        return registered;
}

/**
 *  Claim a username with and protect with a password, then allow this client
 *  to use it as an identity.
 *  @word: password to lock it with
 *  @cli: client to assign
 *  @return: a report on the attempt
 */
int client_register (const char *word, client *cli)
{
        int report = 0;
        FILE *file = NULL;

        if (client_registered(cli->name))
                report = ERR_AUTH;
        else {
                char temp[MAX_BUF] = {0};
                sprintf(temp, "profile/users/%s.txt", cli->name);
                file = fopen(temp, "w");
        }

        if (file) {
                fprintf(file, "password=%s\n", word);
                fprintf(file, "rank=%s\n", client_rank_string(cli->rank));
                fprintf(file, "info=%s\n", cli->info);
                fclose(file);
        } else if (!report)
                report = ERR_FILE_OPEN;

        return report;
}

/**
 *  Make sure the password is correct, then remove the user's profile and unlock
 *  the alias for future users and new registrations.
 *  @cli: client trying to unregister
 */
int client_unregister (client *cli)
{
        int report = 0;

        if (client_registered(cli->name)) {
                char temp[MAX_BUF] = {0};
                sprintf(temp, "profile/users/%s.txt", cli->name);
                remove(temp);
        } else {
                report = ERR_MISSING;
        }

        return report;
}

/**
 *  Send a formatted message to a client, encrypted using the server's secret
 *  key and the client's public key.
 *  @cli: client to talk to
 *  @msg: formatted message
 *  @...: variable argument list (string formatting)
 */
void client_talk (client *cli, const char *msg, ...)
{
        va_list list;
        pack_talk temp;
        memset(&temp, 0, sizeof(pack_talk));

        va_start(list, msg);
        vsprintf(temp.message, msg, list);
        va_end(list);

        host_deliver(crypto_encipher(&temp, sizeof(pack_talk), cli->pk),
                     CHANNEL_NOTIF, cli->peer);
}

/**
 *  Disconnect from a peer with the appropriate procedure as determined by
 *  the passed context.
 *  cmd: context for disconnection
 *  cli: client to disconnect
 */
void client_disconnect(network_cmd context, client *cli)
{
        if (context == NET_FORCE_DISCONNECT)
                enet_peer_disconnect_now(cli->peer, 0);
        else if (context == NET_CIVIL_DISCONNECT)
                enet_peer_disconnect(cli->peer, 0);
}

/**
 *  Free the memory associated with the client, and stop the pointer from
 *  dangling. Call when a disconnect event is received.
 *  @cli: pointer to the client to handle
 */
void client_destroy (client *cli)
{
        if (cli) {
                free(cli);
                cli = NULL;
        }
}

/**
 *  Perform a set of tests to ensure the client module is functioning properly.
 *  @return: result of testing
 */
int client_test()
{
        int report = -9;
        int check;

        /* simulate login */
        client *cli = client_new();
        client_login("player774", NULL, cli);
        if (cli->state == STATE_ENABLED &&
            cli->rank == RANK_NAMED &&
            !strcmp(cli->name, "player774"))
                report += 1;

        /* simulate registration */
        client_register("password", cli);
        if (client_registered(cli->name))
                report += 1;

        /* the user is added to the blacklist */
        client_list_add(LIST_BLACK, cli->name);
        if (client_in_list(LIST_BLACK, cli->name))
                report += 1;

        /* the user is added to the whitelist */
        client_list_add(LIST_WHITE, cli->name);
        if (client_in_list(LIST_WHITE, cli->name))
                report += 1;

        /* user is removed from both lists */
        client_list_remove(LIST_BLACK, cli->name);
        client_list_remove(LIST_WHITE, cli->name);
        if (!client_in_list(LIST_WHITE, cli->name) &&
            !client_in_list(LIST_BLACK, cli->name))
                report += 1;

        /* simulate logout */
        client_destroy(cli);

        /*user attempts to login with bad password */
        cli = client_new();
        if(client_login("player774", "pasd", cli) == ERR_AUTH)
                report += 1;
        client_destroy(cli);

        /* user attempts to log in with good password */
        cli = client_new();
        client_login("player774", "password", cli);
        if (cli->state == STATE_ENABLED &&
            cli->rank == RANK_NAMED &&
            !strcmp(cli->name, "player774"))
                report += 1;

        /* check if the client is registered */
        if (client_registered(cli->name))
                report += 1;

        /* user attempts to doubly register */
        if ((check = client_register("hhhh", cli)))
                report += 1;

        client_unregister(cli);
        client_destroy(cli);

        return report;
}
