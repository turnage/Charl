#include <string.h>
#include "crypto.h"

/* local keypair */
static unsigned char pk[crypto_box_PUBLICKEYBYTES];
static unsigned char sk[crypto_box_SECRETKEYBYTES];

/* current peer's key */
static unsigned char peerk[crypto_box_PUBLICKEYBYTES];

/* server's key (clients only) */
static unsigned char servk[crypto_box_PUBLICKEYBYTES];

/**
 *  See the contents of data represented numerically (rather than a bunch of
 *  weird symbols).
 *  @msg: content to display
 *  @len: how much to display
 */
void see_vals (const void *msg, int len)
{
        int i;
        for (i = 0; i < len; i++)
                printf("%3u ", (*(u8*)(msg + i)));
        printf("\n");
}

/**
 *  Create a keypair
 */
void crypto_init (void)
{
        randombytes_stir();
        crypto_box_keypair(pk, sk);
}

/**
 *  Clients call this when they enter a secure private chat so the crypto module
 *  knows how to properly format private message packets.
 *  @pek: public key of the new peer
 */
void crypto_set_peer (const unsigned char *pek, int mode)
{
        if (mode == PEER_PEER)
                memcpy(peerk, pek, crypto_box_PUBLICKEYBYTES);
        else if (mode == PEER_SERVER)
                memcpy(servk, pek, crypto_box_PUBLICKEYBYTES);
}

/**
 *  Encrypt data without making it a packet.
 *  @src: source of the data
 *  @len: length of the data
 *  @pk: public key to encrypt the data for
 *  @sk: secret key to encrypt the data with
 *  @dest: destination for ciphertext, must be as big as len + PREFIX
 */
void crypto_encipher_data (const void *src, int len, const unsigned char *pek,
                           const unsigned char *sek, void *dest)
{
        unsigned char m[crypto_box_ZEROBYTES + len];

        memset(m, 0, crypto_box_ZEROBYTES + len);
        memset(dest, 0, PREFIX + len);
        memcpy(m + crypto_box_ZEROBYTES, src, len);
        randombytes_buf(dest, crypto_box_NONCEBYTES);

        crypto_box((unsigned char *)dest + crypto_box_NONCEBYTES, m,
                   crypto_box_ZEROBYTES + len, dest, pek, sek);
}

/**
 *  Deciphered data without making it a packet.
 *  @src: source of the data
 *  @len: length of the entire src array
 *  @pk: public key to decrypt the data from
 *  @sk: secret key to decrypt the data with
 *  @dest: destination for plaintext, must be as big as len - PREFIX
 */
int crypto_decipher_data (const void *src, int len, const unsigned char *pek,
                          const unsigned char *sek, void *dest)
{
        int report = 0;
        unsigned char buf[len - crypto_box_NONCEBYTES];
        memset(buf, 0, len - crypto_box_NONCEBYTES);

        report =  crypto_box_open(buf, (const unsigned char *)src +
                                  crypto_box_NONCEBYTES, len -
                                  crypto_box_NONCEBYTES, src, pek, sek);

        memcpy(dest, buf + crypto_box_ZEROBYTES, len - PREFIX);

        return report;
}

/**
 *  Clients call this to make a packet from data before sending it to the server
 *  and servers call it to prepare a packet for a specific client. Clients pass
 *  NULL for the peer parameter.
 *  @data: pointer to the data to encrypt
 *  @len: size of data to encrypt
 *  @peer: only servers use this; it provides the public key to use
 *  @return: a packet prepared with the encrypted data
 */
ENetPacket *crypto_encipher (const void *data, int len, const unsigned char *k)
{
        unsigned char c[PREFIX + len];
        const unsigned char *outkey = k ? k : servk;
        crypto_encipher_data(data, len, outkey, sk, c);

        return enet_packet_create(c, PREFIX + len, ENET_PACKET_FLAG_RELIABLE);
}

/**
 *  Clients call this to decrypt packets received from the server. Servers call
 *  this to decrypt packets received from clients; only servers use the peer
 *  argument.
 *  @pack: packet to decrypt
 *  @plain: place to store the data
 *  @peer: peer from the packet came (servers only)
 */
int crypto_decipher (ENetPacket *pack, void *plain, const unsigned char *k)
{
        int report = 0;
        const unsigned char *outkey = (k) ? k : servk;

        report = crypto_decipher_data(pack->data, pack->dataLength,
                                      outkey, sk, plain);

        return report;
}

/**
 *  Check the length to determine in the packet is encrypted. Try decrypting if
 *  so (and return NULL on failure) but otherwise just return a pointer to the
 *  plaintext.
 *  @pack: packet to break
 *  @k: key to open it with
 *  @return: an error report
 */
char *crypto_make (ENetPacket *pack, const unsigned char *k)
{
        char *mark = NULL;
        int len = crypto_length(pack->dataLength);
        if (len != pack->dataLength) {
                mark = calloc(len, 1);
                if (crypto_decipher(pack, mark, k)) {
                        free(mark);
                        mark = NULL;
                }
        } else {
                mark = calloc(len, 1);
                memcpy(mark, pack->data, len);
        }

        return mark;
}

/**
 *  Determine length of the plaintext stored in a packet.
 *  @len: pack->dataLength
 *  @return: plaintext length
 */
int crypto_length (int len)
{
        return (len > PREFIX) ? len - PREFIX : len;
}

/**
 *  Create a key packet and for the host to deliver to a peer.
 *  @return: packet containing the local public key data
 */
ENetPacket *crypto_getkey (void)
{
        pack_key temp;
        temp.flag = PACK_KEY;
        memcpy(temp.pk, pk, crypto_box_PUBLICKEYBYTES);
        
        return enet_packet_create(&temp, sizeof(pack_key),
                                  ENET_PACKET_FLAG_RELIABLE);
}

/**
 *  Close the random file descriptor.
 */
void crypto_close (void)
{
        randombytes_close();
}

/**
 *  Test all of the functions in the crypto module.
 *  @return: result of testing
 */
int crypto_test (void)
{
        int checkpoints = -5;

        /* testing keypair a */
        unsigned char pka[crypto_box_PUBLICKEYBYTES] = {0};
        unsigned char ska[crypto_box_SECRETKEYBYTES] = {0};
        crypto_box_keypair(pka, ska);

        /* testing keypair b */
        unsigned char pkb[crypto_box_PUBLICKEYBYTES] = {0};
        unsigned char skb[crypto_box_SECRETKEYBYTES] = {0};
        crypto_box_keypair(pkb, skb);

        /* message to test crypto with */
        char message[] = "Hello";

        /* test of encipher/decipher of data */
        {
                unsigned char plain[5] = {0};
                unsigned char dest[PREFIX + 5];

                /* data was encrypted */
                crypto_encipher_data(message, 5, pkb, ska, dest);
                if (memcmp((char *)dest + PREFIX, message, 5))
                        checkpoints += 1;

                /* data was decrypted */
                crypto_decipher_data((char *)dest, PREFIX + 5, pka, skb, plain);
                if (!memcmp(plain, message, 5))
                        checkpoints += 1;

        }

        /* test of encipher/decipher packets */
        {

                unsigned char plain[5] = {0};

                /* give module the identity of constituent a */
                memcpy(sk, ska, crypto_box_SECRETKEYBYTES);

                /* give server the identity of constituent b */
                memcpy(servk, pkb, crypto_box_PUBLICKEYBYTES);

                /* data was encrypted */
                ENetPacket *pack = crypto_encipher(message, 5, NULL);
                if (pack)
                        if (pack->dataLength == PREFIX + 5)
                                if (memcmp(pack->data + PREFIX, message, 5))
                                        checkpoints += 1;

                /* give module the identity of constituent b */
                memcpy(sk, skb, crypto_box_SECRETKEYBYTES);

                /* give server the identity of constituent a */
                memcpy(servk, pka, crypto_box_PUBLICKEYBYTES);

                /* data was decrypted */
                crypto_decipher(pack, plain, NULL);
                if (!memcmp(plain, message, 5))
                       checkpoints += 1;

               enet_packet_destroy(pack);

        }

        if (crypto_getkey())
                checkpoints += 1;

        return checkpoints;
}
