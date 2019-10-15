CC=gcc
CFLAGS=-Wall -g
LFLAGS=-I./include -g -Wall
EXECUTABLE=chat
SRC=$(addprefix src/, c_chat.c chat_socket.c client.c server.c)
OBJ=$(SRC:.c=.o)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) -lrt $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(LFLAGS) -c $< -o $@

clear: clean

clean:
	rm -fv $(OBJ) $(EXECUTABLE)
