CC=gcc
LLIB=-pthread -lssl -lcrypto -lcurses
CFLAGS=-g -Wall $(LLIB)
LFLAGS=-Iinclude -g -Wall
EXECUTABLE=chat
SRC=$(addprefix src/, chat.c chat_socket.c client.c server.c log.c client_ui.c)
OBJ=$(SRC:.c=.o)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(LFLAGS) -c $< -o $@

clear: clean

clean:
	rm -fv $(OBJ) $(EXECUTABLE)
