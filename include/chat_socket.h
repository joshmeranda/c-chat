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
typedef struct sock_info SOCK;

struct sock_info
{
    struct sockaddr_in addr;
    int fd; // file descriptor
    SSL *ssl;
    SSL_CTX *ctx;
    char buffer[BUFFER_SIZE];
};

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

#endif // CHAT_SOCKET_H