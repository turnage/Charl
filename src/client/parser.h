#ifndef PARSER_H
#define PARSER_H

#include <charl.h>
#include <gtk/gtk.h>

/* Handle a local command. */
int parser_out (const char *line);

/* Handle a remote command. */
int parser_in ();

/* Validate a connect command. */
int parser_out_connect (const char *line);

/* Validate a login command. */
int parser_out_login (const char *line);

/* Validate a hop command. */
int parser_out_hop (const char *line);

/* Validate a registration command. */
int parser_out_register (const char *line);

/* Validate a logout command. */
int parser_out_logout ();

/* Validate an unregistration command. */
int parser_out_unregister ();

/* Validate a stop command. */
int parser_out_stop ();

/* Create an id packet. */
ENetPacket *parser_pack_alias (const char *alias, const char *word);

/* Create a message packet */
ENetPacket *parser_pack_talk (const char *msg);

/* Handle a packet on the auth channel. */
int parser_auth (void *data, int len);

/* Make sure an address string is in the correct format. */
int parser_check_address (const char *address);

#endif /* PARSER_H */