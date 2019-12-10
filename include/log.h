#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define LOG_T   \
    X(JOIN)     \
    X(LEAV)     \
    X(RJCT)     \
    X(FULL)     \
    X(STRT)     \
    X(STOP)     \
    X(BDRD)     \
    X(BDWR)     \
    X(CERR)

/**
 * Specifies the type / code for a long entry. The possible values and their
 * meanings are as follows:
 *      JOIN : (join)       A client has been successfully joined the server.
 *      LEAV : (leave)      A client has successfully left the server.
 *      RJCT : (reject)     A client has attempted to join the server with a
 *                          non-unique username.
 *      FULL : (full)       A client has attempted to join a server which has
 *                          Already reached it max_client limit.
 *      STRT : (start)      A server session has begun.
 *      STOP : (stop)       A server session has completed.
 *      BDRD : (bad read)   The server has encountered an error when reading
 *                          from a file descriptor.
 *      NDWR : (bad write)  The server has encountered an error when writing to
 *                          a file descriptor.
 *      CERR : (chat error) Some error has been encountered which does not have
 *                          its own log code.
 */
typedef enum {
#define  X(val) val,
    LOG_T
#undef X
} log_t;

/**
 * Generate the timestamp for a log event. Will be in the format
 * [dd-mm-yyy hh:mm:ss], and so stamp parameter must be 22 characters long
 * as to hold the entire stamp and the null byte.
 *
 * @param stamp The string to contain the time stamp.
 * @return The timestamp.
 */
char* get_timestamp(char *stamp);

/**
 * Get a log string from the associated LOG_CODE enum value. If there is no
 * matching value for the log_t, "UNKN" (aka UNKOWN) is returned.
 *
 * @param code The code from which to find the string.
 * @return The string representation of the passed code.
 */
char* get_log_str(log_t code);

/**
 * Append a client event log entry with the given code.
 *
 * @param log The file stream for the log file.
 * @param code The log code for the log entry.
 * @param address The ip of the connection.
 * @param port The port of the connection.
 * @param username The username of the connection.
 */
void client_event_log_entry(FILE *log, log_t code, char *address, int port, char *username);

/**
 * Append a server event log entry with the given code.
 *
 * @param log The file stream for the log file.
 * @param code The loge code for the log entry.
 * @param address The ip of the connection.
 * @param port The port of the connection.
 */
void server_event_log_entry(FILE *log, log_t code, char *address, int port, char *msg);

#endif // LOG_H