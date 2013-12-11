#include <stdio.h>
#include <string.h>

#include <charl.h>
#include <crypto.h>
#include <host.h>

#include "client.h"
#include "parser.h"

#define _SERVER

/* test the modules before starting so errors can be diagnosed */
int test (void);

/* prepare the server and its resources */
int init (void);

/* the primary loop */
void run (void);

/* parse packets */
void handle_event (ENetEvent *ev);

/* free all resources */
void destroy (void);

int main (void)
{
        int x;

        if ((x = test()))
                printf("Module tests failed. Aborting [%d].\n", x);
        else if ((x = init()))
                printf("Failed to initialize. Aborting [%d].\n", x);
        else {
                run();
                destroy();
        }

        return 0;
}

/**
 *  Call module tests with verbose reporting.
 */
int test (void)
{
        int report, count = 0;

        printf("\nPerforming tests\n");
        printf("-----------------------------------------------------------\n");

        printf("Crypto module......................................");
        if (!(report = crypto_test()))
                printf("....[OK]\n");
        else {
                printf("[%6d]\n", report);
                count += 1;
        }

        printf("Client module......................................");
        if (!(report = client_test()))
                printf("....[OK]\n");
        else {
                printf("[%6d]\n", report);
                count += 1;
        }

        printf("Parser module......................................");
        if (!(report = parser_test()))
                printf("....[OK]\n");
        else {
                printf("[%6d]\n", report);
                count += 1;
        }

        printf("-----------------------------------------------------------\n");

        return count;
}

/**
 *  Parse the config file, and initiate the host and crypto modules.
 */
int init (void)
{
        int port, max, capup, capdown;
        parser_load_config(&port, &max, &capup, &capdown);
        crypto_init();
        return host_init_server(port, max, capup, capdown);
}

/**
 *  The primary loop and logic. Listen for packets and process them accordingly.
 */
void run (void)
{
        printf("Server running normally...\n");

        ENetEvent *ev = NULL;
        client *cli = NULL;
        u64 nullpacks = 0;
        connected = 1;

        do {
                ev = host_listen(500);

                if (!ev) {
                        nullpacks++;
                } else if (ev->type == ENET_EVENT_TYPE_CONNECT) {
                        ev->peer->data = client_new();
                        cli = ev->peer->data;
                        cli->peer = ev->peer;
                        fprintf(stderr, "Client connected.\n");
                } else if (ev->type == ENET_EVENT_TYPE_RECEIVE) {
                        fprintf(stderr, "Packet received.\n");
                        handle_event(ev);
                        enet_packet_destroy(ev->packet);
                } else if (ev->type == ENET_EVENT_TYPE_DISCONNECT) {
                        cli = ev->peer->data;
                        host_list(cli->channel);
                        client_destroy(cli);
                        fprintf(stderr, "Client left.\n");
                } else {
                        fprintf(stderr, "Unrecognized event.\n");
                }

        } while (connected);
}

/**
 *  Parse a packet received from a client and delegate processing appropriately.
 *  @ev: event to parse
 */
void handle_event (ENetEvent *ev)
{
        int len = crypto_length(ev->packet->dataLength);
        void *data = crypto_make(ev->packet,
                                 ((client *)ev->peer->data)->pk);

        fprintf(stderr, "Packet on channel %d.\n", ev->channelID);

        if (data) {
                /* identification and authentication packets */
                if (ev->channelID == CHANNEL_AUTH)
                        parser_auth(data, len, ev->peer);
                else if (ev->channelID == CHANNEL_PUBLIC)
                        parser_public(data, len, ev->peer);
                else if (ev->channelID == CHANNEL_MOD)
                        parser_mod(data, len, ev->peer);
                else if (ev->channelID == CHANNEL_DATA)
                        parser_data(data, len, ev->peer);
                else if (ev->channelID == CHANNEL_PRIVATE)
                        parser_privchat(data, len, ev->peer);
                free(data);
        } else
                printf("Decryption failed.\n");
}

/**
 *  Destroy the host and all connected peers.
 */
void destroy (void)
{
        host_destroy();
        crypto_close();
}
