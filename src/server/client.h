#ifndef CLIENT_H
#define CLIENT_H

#include <charl.h>

/* Functions to verbally represent enumerations. */
const char *client_rank_string(rank_id rank);
const char *client_state_string(state_id state);
const char *client_list_string (list_id list);

/* Check to see if a user is part of a list. */
int client_in_list (list_id list, const char *name);

/* Add a client to a list. */
void client_list_add (list_id list, const char *name);

/* Remove a client from a list. */
void client_list_remove (list_id list, const char *name);

/* Create a new client with the default values (uses malloc). */
client *client_new ();

/* Assign a client an address to communicate with. */
void client_assign (ENetPeer *peer, client *cli);

/* Load a client's information from file. */
int client_load (const char *name, const char *word, client *cli);

/* Assign a client an alias. */
int client_login(const char *name, const char *word, client *cli);

/* Print the client's information to the screen. */
void client_report (client cli);

/* See if a client has a profile. */
int client_registered (const char *name);

/* Save a client's information to file. */
int client_register (const char *word, client *cli);

/* Remove a client's profile file, and unlock the alias. */
int client_unregister (client *cli);

/* Relay a message to the client, encrypted. */
void client_talk (client *cli, const char *msg, ...);

/* Disconnect from a client. */
void client_disconnect(network_cmd context, client *cli);

/* Free a client's memory; must follow client_new calls. */
void client_destroy (client *cli);

/* Test the client module. */
int client_test();

#endif /* CLIENT_H */
