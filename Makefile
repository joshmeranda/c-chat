CC=gcc
CFLAGS=-g -Wall -pthread
LFLAGS=-I./include -g -Wall
EXECUTABLE=chat
SRC=$(addprefix src/, chat.c chat_socket.c client.c server.c)
OBJ=$(SRC:.c=.o)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(LFLAGS) -c $< -o $@

clear: clean

clean:
	rm -fv $(OBJ) $(EXECUTABLE)
