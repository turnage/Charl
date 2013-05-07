#ifndef CHARL_H
#define CHARL_H

#include <sodium.h>
#include <enet/enet.h>
#include <inttypes.h>

#ifdef _WIN32
        #include <win.h>
#endif

#define PREFIX (crypto_box_NONCEBYTES + crypto_box_ZEROBYTES)

#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat"

/* Unsigned proxies. */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* Global links. */
extern int connected;
extern int logged_in;

/* Size limits. */
typedef enum max_id {

        MAX_BUF  = 512,

        MIN_NAME = 3,
        MAX_NAME = 11,

        MAX_CHAN = 10,

        MIN_PASS = 5,
        MAX_PASS = 50,

        MAX_TRIES = 3

} max_id;

/* Error codes. */
typedef enum error_id {

        ERR_AUTH      = 1,
        ERR_MISSING   = 2,
        ERR_INVALID   = 3,
        ERR_FILE_OPEN = 4,
        ERR_ENET      = 5,
        ERR_PORTBIND  = 6

} error_id;

/* Channel reservations. */
typedef enum chan_id {

        CHANNEL_AUTH    = 0,
        CHANNEL_NOTIF   = 1,
        CHANNEL_DATA    = 2,
        CHANNEL_INVITE  = 3,
        CHANNEL_PRIVATE = 4,
        CHANNEL_PUBLIC  = 5,
        CHANNEL_MOD     = 6

} chan_id;

/* Moderator operations. */
typedef enum mod_id {

        MOD_STOP        = 0,
        MOD_MUTE        = 1,
        MOD_UNMUTE      = 2,
        MOD_ISOLATE     = 3,
        MOD_UNISOLATE   = 4,
        MOD_KICK        = 5,
        MOD_BAN         = 6,
        MOD_RANK        = 7,
        MOD_UNREGISTER  = 8,
        MOD_BLACKLIST   = 9,
        MOD_UNBLACKLIST = 10,
        MOD_WHITELIST   = 11,
        MOD_UNWHITELIST = 12

} mod_id;

/* Requests clients can send the server. */
typedef enum request_id {

        REQUEST_LIST = 0,
        REQUEST_INFO = 1,
        REQUEST_KEY  = 2

} request_id;

/* Packet types. */
typedef enum pack_id {

        PACK_KEY      = 0,  /* public key exchanges */
        PACK_ALIAS    = 1,  /* logins, registration, adding info, etc */
        PACK_MOD      = 2,  /* change a user's state, or rank, kick, mute */
        PACK_REQUEST  = 4,  /* request for a list of users, a user's info */
        PACK_TALK     = 5,  /* server sending a notification, or user chat */
        PACK_INVITE   = 6,  /* a peer's invitation to a private chat */
        PACK_PRIVCHAT = 7,  /* packet for relaying (not reading) */
        PACK_LIST     = 8,  /* a list of peers on the current channel */
        PACK_LOGOUT   = 9,  /* a request to have the user session ended */
        PACK_UNREG    = 10, /* a request to have the user profile removed */
        PACK_CHAN     = 11  /* a request to change the virtual channel */

} pack_id;

/* Rank lists. */
typedef enum rank_id {

        RANK_NONE      = 0,
        RANK_NAMED     = 1,
        RANK_MODERATOR = 2,
        RANK_ADMIN     = 3

} rank_id;

/* Talking states. */
typedef enum state_id {

        STATE_DISABLED = 0,
        STATE_ENABLED  = 1,
        STATE_MUTED    = 2,
        STATE_ISOLATED = 3

} state_id;

/* Network commands. */
typedef enum network_cmd {

        NET_FORCE_DISCONNECT = 1,
        NET_CIVIL_DISCONNECT = 2

} network_cmd;

/* User lists. */
typedef enum list_id {

        LIST_BLACK = 1,
        LIST_WHITE = 2

} list_id;

/* Host types. */
typedef enum host_id {

        HOST_SERVER = 1,
        HOST_CLIENT = 2

} host_id;

/* Peer markers. */
typedef enum peer_id {

        PEER_SERVER = 0,
        PEER_PEER   = 1

} peer_id;

/* Colors for the log. */
typedef enum color_id {

        COL_NONE  = 0,
        COL_NOTIF = 1,
        COL_ERROR = 2,
        COL_PRIV  = 3,
        COL_ANON  = 4,
        COL_NAME  = 5,
        COL_MOD   = 6,
        COL_ADMIN = 7

} color_id;

/* Client descriptor. */
typedef struct client {

        rank_id rank;
        state_id state;
        u32 channel;
        char name[MAX_NAME];
        char info[MAX_BUF];
        unsigned char pk[crypto_box_PUBLICKEYBYTES];
        ENetPeer *peer;

} client;

/* Session configuration. */
typedef struct session_data {

        char ready;
        ENetAddress address;
        char add[MAX_BUF];
        char alias[MAX_NAME];
        char word[MAX_PASS];

} session_data;

/* Packet structure definitions. */
typedef struct pack_key {

        pack_id flag;
        unsigned char pk[crypto_box_PUBLICKEYBYTES];

} pack_key;

typedef struct pack_alias {

        pack_id flag;
        char alias[MAX_NAME];
        char word[MAX_PASS];

} pack_alias;

typedef struct pack_mod {

        pack_id flag;
        mod_id operation;
        char alias[MAX_NAME];

} pack_mod;

typedef struct pack_chan {

        pack_id flag;
        int channel;
        char word[MAX_PASS];

} pack_chan;

typedef struct pack_request {

        pack_id flag;
        char alias[MAX_NAME];
        request_id request;

} pac_request;

typedef struct pack_talk {

        pack_id flag;
        char message[MAX_BUF];

} pack_talk;

typedef struct pack_invite {

        pack_id flag;
        char sender_alias[MAX_NAME];
        char recip_alias[MAX_NAME];
        unsigned char pk[crypto_box_PUBLICKEYBYTES];

} pack_invite;

typedef struct pack_privchat {

        pack_id flag;
        char alias[MAX_NAME];
        char message[MAX_BUF];

} pack_privchat;

#endif /* CHARL_H */