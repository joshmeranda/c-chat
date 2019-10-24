#include "log.h"
#include <time.h>
#include <string.h>

char *get_timestamp(char *stamp)
{
    time_t t = time(NULL);
    struct tm *time = localtime(&t);
    memset(stamp, 0, 22);

    sprintf(stamp, "[%02d-%02d-%04d %02d:%02d:%02d]", time->tm_mday, time->tm_mon + 1,
            time->tm_year + 1900, time->tm_hour, time->tm_min, time->tm_sec);

    return stamp;
}

char* get_log_str(log_t code)
{
    switch (code)
    {
        #define X(code)     \
            case code:      \
                return #code;
                LOG_T
        #undef X
        default:
            return "UNKN";
    }
}

void client_event_log_entry(FILE *log, log_t code, char *address, int port, char *username)
{
    char timestamp[22];

    get_timestamp(timestamp);
    fprintf(log, "%s %s %s:%d %s\n", timestamp, get_log_str(code), address, port, username);
    printf("%s %s %s:%d %s\n", timestamp, get_log_str(code), address, port, username);
    fflush(log);
}

void server_event_log_entry(FILE *log, log_t code, char *address, int port, char *msg)
{
    char timestamp[22];

    get_timestamp(timestamp);
    fprintf(log, "%s %s %s:%d %s\n", timestamp, get_log_str(code), address, port, msg);
    printf("%s %s %s:%d %s\n", timestamp, get_log_str(code), address, port, msg);
    fflush(log);
}