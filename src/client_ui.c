#include "client_ui.h"
#include <string.h>
#include <malloc.h>
#include <curses.h>

void update_data(wdata_t *data, char *message)
{
    int index;

    // initial full
    if (data->count == data->size - 1)
    {
        index = data->offset;
        data->offset++;
        free(data->data[index]);
        data->data[index] = NULL;
    }
    else
    {
        index = data->count;
    }

    data->data[index] = strdup(message);
    data->count++;
}

void update_screen(wdata_t *data)
{
    clear();

    int i = data->offset;
    do
    {
        if (! data->data[i]) break;

        // print packet sections
        char *src = get_next_section(&data->data[i]);
        char *msg = data->data[i] + strlen(src) + strlen(DELIMITER);
        msg = get_next_section(&msg);

        if (! *msg)
        {
            printw("%s\n", src);
        }
        else    // client messages (ie invalid such command)
        {
            printw("%s : %s\n", src, msg);
        }

        refresh();

        i = (i == data->size - 1) ? 0 : i + 1;
    } while (i != data->offset);

    move(getmaxy(stdscr) - 1, 0); // move cursor to the bottom of the screen
    refresh();
}