#ifndef PARSER_H
#define PARSER_H

#include "charl.h"

/* Load the main config file. */
void parser_load_config (int *port, int *max, int *capup, int *capdown);

/* Load channel locks. */
void parser_load_channels (int channel, char *word);

/* Handle a packet on the auth channel. */
void parser_auth (const void *data, int len, ENetPeer *peer);

/* Handle a packet on the public channel. */
void parser_public (const void *data, int len, ENetPeer *peer);

/* Handle a packet on the moderation channel. */
void parser_mod (const void *data, int len, ENetPeer *peer);

/* Handle a packet on the data channel. */
void parser_data (const void *data, int len, ENetPeer *peer);

/* Handle a packet on the private chat channel. */
void parser_privchat (const void *data, int len, ENetPeer *peer);

/* Assign a client the key the sent. */
void parser_key (pack_key *pack, client *cli);

/* Appropriately handle an aliasing attempt. */
void parser_alias (pack_alias *pack, client *cli);

/* Test the parser module. */
int parser_test ();

#endif /* PARSER_H */
