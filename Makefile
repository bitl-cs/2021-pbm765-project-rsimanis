SERVER_PROGRAM = server.c
SERVER_EXECUTABLE = server

CLIENT_PROGRAM = client.c
CLIENT_EXECUTABLE = client

CFLAGS = -std=gnu11 -fno-common -Wall -Wextra
CFILES_SERVER = $(SERVER_PROGRAM) args.c pong_server.c pong_networking.c pong_math.c pong_game.c -lpthread
CFILES_CLIENT = $(CLIENT_PROGRAM) args.c pong_client.c pong_networking.c pong_math.c pong_game.c -lpthread

all: $(SERVER_EXECUTABLE) $(CLIENT_EXECUTABLE)

$(SERVER_EXECUTABLE): $(CFILES_SERVER)
	@gcc $(CFLAGS) -o $(SERVER_EXECUTABLE) $(CFILES_SERVER) 

$(CLIENT_EXECUTABLE): $(CFILES_CLIENT)
	@gcc $(CFLAGS) -o $(CLIENT_EXECUTABLE) $(CFILES_CLIENT) 

.PHONY: clean

clean: 
	rm $(SERVER_EXECUTABLE) $(CLIENT_EXECUTABLE)
