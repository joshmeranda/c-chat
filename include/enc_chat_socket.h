#ifndef C_CHAT_ENC_CHAT_SOCKET_H
#define C_CHAT_ENC_CHAT_SOCKET_H

#include <openssl/ssl.h>
#include <netinet/in.h>

#define USERNAME_MAX 25
#define BUFFER_SIZE USERNAME_MAX * 2 + 3 // enough to span <src>|<dest>|<256 chars>|
#define DELIMITER "|"

// http://simplestcodings.blogspot.com/2010/08/secure-server-client-using-openssl-in-c.html - see for tls example

/**
 * Check the validity of the cert and key files, and adjust the context
 * to account for them.
 *
 * @param ctx The context to be modified with the cret and key files.
 * @param cert The path to the cert file to validate.
 * @param key The path to the key file to validate.
 */
void load_certs(SSL_CTX *ctx, char *cert, char *key);

/**
 * Initiate the ssl context for the client.
 *
 *
 * @return Pointer to the initiated ssl context.
 */
SSL_CTX* init_server_ctx();

/**
 * Initiate the ssl context for the server.
 *
 * @return Pointer to the initiated ssl context.
 */
SSL_CTX* init_client_ctx();

/**
 * Read content from an ssl connection.
 *
 * @param ssl The ssl connect from which to read.
 * @param buffer The buffer to read data into.
 * @return The amount of bytes which have been read from the ssl connection.
 */
ssize_t read_ssl(SSL *ssl, char *buffer);

/**
 * Write content to a ssl connection.
 *
 * @param ssl The ssl connect to which data is to be written.
 * @param message The message to write to the ssl connection.
 * @return The amount of bytes which was written to the ssl connection.
 */
ssize_t send_ssl(SSL *ssl, char *message);

#endif //C_CHAT_ENC_CHAT_SOCKET_H
