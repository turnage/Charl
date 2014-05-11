#include <string.h>
#include <stdio.h>

#include "host.h"
#include "crypto.h"

static ENetEvent ev;
static ENetAddress address;
static ENetHost *host;
static ENetPeer *peer;

static const u32 timeout = 20;

/**
 *  Create and initialize the server host to handle networking.
 *  port: port for the server to listen on
 *  max: maximum clients to server
 *  capup: maximum upstream bandwidth
 *  capdown: maximum downstream bandwidth
 *  @return: success or failure
 */
int host_init_server (int port, int max, int capup, int capdown)
{
        int report = 0;

        address.host = ENET_HOST_ANY;
        address.port = port;

        if (enet_initialize())
                report = ERR_ENET;
        else {
                host = enet_host_create(&address, max, 7, capup, capdown);
                if (!host)
                        report = ERR_PORTBIND;
        }

        return report;
}

/**
 *  Create a clientside host and initialize the enet module.
 *  @return: success (0) or failure (1)
 */
int host_init_client (void)
{
        if (!enet_initialize())
                host = enet_host_create(NULL, 1, 7, 0, 0);
        return host ? 0 : 1;
}

/**
 *  Establish a tracked connection with a peer.
 *  @address: address of peer
 *  @return: 0 for successful, 1 for failure
 */
int host_connect (ENetAddress *address)
{
        int report = 1;
        ENetEvent *confirm = NULL;
        peer = enet_host_connect(host, address, 7, 0);
        if ((confirm = host_listen(200)))
                if (confirm->type == ENET_EVENT_TYPE_CONNECT) {
                        report = 0;
                        connected = 1;
                }
        return report;
}

/**
 *  Destroy any connections to peers and reset the peer metadata.
 */
void host_disconnect (void)
{
        enet_peer_disconnect(peer, 0);
        host_listen(0);
        connected = 0;
}

/**
 *  Deliver data to the primary (or otherwise provided) peer.
 *  @pack: data to deliver
 *  @chan: channel to deliver on
 *  @dest: destination (leave NULL for default peer)
 *  @return: success or failure of send attempt
 */
int host_deliver (ENetPacket *pack, chan_id chan, ENetPeer *dest)
{
        ENetPeer *outpeer = (dest) ? dest : peer;
        int report = enet_peer_send(outpeer, chan, pack);
        return report;
}

/**
 *  Broadcast a packet to all users on the provided virtual channel. Forward
 *  -1 in the channel argument to send to all channel without discrimination.
 *  @data: data to broadcast
 *  @len: length of the data
 *  @chan: physical channel to use
 *  @channel: virtual channel to filter with
 */
void host_broadcast (const void *data, int len, chan_id chan, int channel)
{
        int i = 0;
        for (i = 0; i < host->peerCount; i++) {
                if ((host->peers[i].state == ENET_PEER_STATE_CONNECTED)       &&
                    (((client*)host->peers[i].data)->state != STATE_DISABLED) &&
                    (((client*)host->peers[i].data)->state != STATE_ISOLATED) &&
                    ((((client*)host->peers[i].data)->channel == channel)     ||
                    (channel == -1))) {
                        client *cli = host->peers[i].data;
                        host_deliver(crypto_encipher(data, len, cli->pk),
                                     chan, cli->peer);
                }
        }
}

/**
 *  Search the host's connected peer list for a user by name, and return that
 *  peer's ENetPeer.
 *  @name: name to search for
 *  @return: that user's peer, or NULL
 */
ENetPeer *host_find_peer (const char *name)
{
        int i = 0;
        ENetPeer *peer = NULL;
        client *cli = NULL;

        while ((i < host->peerCount) && !peer) {
                if (host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
                        cli = host->peers[i].data;
                        if (!strcmp(cli->name, name))
                                peer = &host->peers[i];
                }
                i += 1;
        }

        return peer;
}

/**
 *  Search through the peers connected to the server to find ones on the
 *  provided virtual channel. Compile those into a list with newline
 *  delimiters for the clients to display locally as a peer list.
 *  @channel: virtual channel to search
 *  @return: a compiled encrypted packet
 */
int host_list (int channel)
{
        int i = 0, report = 0;
        pack_talk temp;
        client *cli = NULL;
        ENetPacket *pack = NULL;

        memset(&temp, 0, sizeof(pack_talk));
        temp.flag = PACK_LIST;

        for (i = 0; i < host->peerCount; i++) {
                if (host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
                        cli = host->peers[i].data;
                        if ((cli->state == STATE_ENABLED) &&
                            (cli->channel == channel))
                                sprintf(temp.message + strlen(temp.message),
                                        "%s\n", cli->name);
                        else if (cli->channel == channel)
                                sprintf(temp.message + strlen(temp.message),
                                        "[Anonymous]\n");
                }
        }

        for (i = 0; i < host->peerCount; i++) {
                if (host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
                        cli = host->peers[i].data;
                        if (cli->channel == channel) {
                                pack = crypto_encipher(&temp, sizeof(pack_talk),
                                                       cli->pk);
                                report = host_deliver(pack, CHANNEL_DATA,
                                                      cli->peer);
                        }
                }
        }

        return report;
}

/**
 *  Scan for incoming packets and events, return them to be handled externally.
 *  @time: time to listen for packets (0 for default)
 *  @return: event or NULL
 */
ENetEvent *host_listen (int timer)
{
        ENetEvent *ret = &ev;
        int usedtime = timer ? timer : timeout;
        int report = enet_host_service(host, &ev, usedtime);
        if(!report)
                ret = NULL;
        else if (report < 0)
                fprintf(stderr, "Network error.\n");
        return ret;
}

/**
 *  Destroy the host and all connected peers.
 */
void host_destroy (void)
{
        int i = 0;

        /* deallocate all peers */
        for (i = 0; i < host->peerCount; i++) {
                if (host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
                        if (host->peerCount > 1)
                                free(host->peers[i].data);
                        enet_peer_disconnect_now(&host->peers[i], 0);
                }
        }

        /* deallocate the host */
        enet_host_destroy(host);
}
