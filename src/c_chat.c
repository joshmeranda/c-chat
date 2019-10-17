#include "chat_socket.h"
#include "client.h"
#include "server.h"

#include <error.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

/**
 * Enum for the possible modes for the chat.
 */
enum Mode { SERVER, CLIENT };

int main(int argc, char** argv)
{
    enum Mode mode = CLIENT;
    char* address = "127.0.0.1",
          *username = "GUEST";
    int port = 8080,
        arg;

    struct option long_options[] = {
            {"address",  required_argument, 0, 'a'},
            {"port",     required_argument, 0, 'p'},
            {"username", required_argument, 0, 'u'},
            {0,         0,                 0, 0}
    };

    while ((arg = getopt_long(argc, argv, "a:p:u:", long_options, 0)) > 0)
    {
        switch (arg)
        {
            case 'a':
                address = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'u':
                username = optarg;
                break;
            default:
                return 1; // Stop execution upon unknown command
        }
    }

    for (; optind < argc; optind++)
    {
        if (strcmp(argv[optind], "client") == 0)
        {
            mode = CLIENT;
        } else if (strcmp(argv[optind], "server") == 0)
        {
            mode = SERVER;
        } else {
            error(0, 0, "unknown option '%s'\n", argv[optind]);
        }
    }

    switch (mode)
    {
        case SERVER:
            run_server(address, port);
            break;
        case CLIENT: // CLIENT is the default mode
        default:
            run_client(address, port, username);
            break;
    }
}