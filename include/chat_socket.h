#ifndef CHAT_SOCKET_H
#define CHAT_SOCKET_H

#define USERNAME_MAX 25
#define BUFFER_SIZE USERNAME_MAX * 2 + 3 // enough to span <src>|<dest>|<256 chars>|
#define DELIMITER "|"

#include <netinet/in.h>
#include <openssl/ssl.h>

/**
 * Describes a connected socket.
 */
typedef struct
{
    struct sockaddr_in addr;
    int fd;
    SSL *ssl;
    SSL_CTX *ctx;
    char buffer[BUFFER_SIZE];
} sock_t;

int exit_received;

/**
 * Set the value of rec_signal to indicate that a signal has been received
 * by the program.
 *
 * @param sig The received signal.
 */
void handle_signal(int sig);

/**
 * Read data from a file descriptor.
 *
 * @param fd The file descriptor to read dat from.
 * @param buffer The buffer to store the data in.
 * @return The amount of characters read into the buffer, or -1 on error.
 */
ssize_t read_fd(int fd, char *buffer);

/**
 * Write data to a file descriptor.
 *
 * @param fd The file descriptor to write to.
 * @param message The data to write to the file descriptor.
 * @return The amount of data written to the file descriptor, or -1 on error.
 */
ssize_t send_fd(int fd, char* message);

/**
 * Get the next word in a string, meaning there is a ' ' or '\n' ending the
 * string.
 *
 * @param input The string to parse.
 * @param term The possible terminating characters for the next word.
 * @return Pointer to the null terminated word, needs to be freed after use.
 */
char *get_next_word(char *input, char *term);

/**
 * Shutdown a file descriptor.
 *
 * @param fd The file descriptor to shut down.
 * @return 0 if successful, -1 if failure.
 */
int shutdown_fd(int fd);

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

ssize_t chat_send(int fd, SSL *ssl, int enc, char *packet);

ssize_t chat_read(int fd, SSL *ssl, int enc, char *buffer);

#endif // CHAT_SOCKET_H