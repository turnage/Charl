#ifndef CRYPTO_H
#define CRYPTO_H

#include <charl.h>

/* Preview data for debug. */
void see_vals (const void *msg, int len);

/* Initialize the crypto module with the server's public key. */
void crypto_init ();

/* Prepare the crypto module for making private chat messages. */
void crypto_set_peer (const unsigned char *pek, int mode);

/* Encrypt data without making it into a packet. */
void crypto_encipher_data (const void *src, int len, const unsigned char *pek,
                           const unsigned char *sek, void *dest);

/* Decrypt data without extracting it from a packet. */
int crypto_decipher_data (const void *src, int len, const unsigned char *pek,
                          const unsigned char *sek, void *dest);

/* Create a packet from the passed data; use context to infer size. */
ENetPacket *crypto_encipher (const void *data, int len, const unsigned char *k);

/* Decrypt a packet's data and store it in the plain array. */
int crypto_decipher (ENetPacket *pack, void *plain, const unsigned char *k);

/* Interpret a packet, decrypt it if it is encrypted, and return a pointer. */
char *crypto_make (ENetPacket *pack, const unsigned char *k);

/* Determine size of actual data. */
int crypto_length (int len);

/* Construct a public key packet. */
ENetPacket *crypto_getkey ();

/* Close the random file descriptor. */
void crypto_close();

/* Test the crypto module and all of its functions. */
int crypto_test ();

#endif /* CRYPTO_H */
