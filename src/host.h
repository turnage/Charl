#ifndef HOST_H
#define HOST_H

#include <charl.h>

/* Set up the server host. */
int host_init_server (int port, int max, int capup, int capdown);

/* Set up the client host. */
int host_init_client ();

/* Establish connection to a peer. */
int host_connect (ENetAddress *address);

/* Destroy a connection to any peers. */
void host_disconnect ();

/* Send a packet to the primary (or provided) peer. */
int host_deliver (ENetPacket *pack, chan_id chan, ENetPeer *dest);

/* Broadcast a packet to all users on the given channel. */
void host_broadcast (const void *data, int len, chan_id chan, int channel);

/* Return a peer, found by name. */
ENetPeer *host_find_peer (const char *name);

/* Construct a list of the peers in a given channel, for a given client. */
int host_list (int channel);

/* Listen for events and return one if received. */
ENetEvent *host_listen (int channel);

/* Destroy the host. */
void host_destroy ();

#endif /* HOST_H */
