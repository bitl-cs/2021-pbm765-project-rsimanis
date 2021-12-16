SERVER_MAIN = server.c
SERVER_EXE = server

CLIENT_MAIN = client.c
CLIENT_EXE = client

CFLAGS = -std=gnu90 -fno-common -Wall -Wextra
LDFLAGS = -lpthread -lGL -lGLU -lglut -lm

CFILES_BOTH = args.c pong_networking.c pong_math.c pong_game.c
CFILES_SERVER = $(CFILES_BOTH) pong_server.c $(SERVER_MAIN)
CFILES_CLIENT = $(CFILES_BOTH) pong_client.c message_list.c graphics.c $(CLIENT_MAIN)

all: $(SERVER_EXE) $(CLIENT_EXE)
graphics: graphics.exe

$(SERVER_EXE): $(CFILES_SERVER)
	@gcc $(CFLAGS) $(LDFLAGS) -o $(SERVER_EXE) $(CFILES_SERVER)

$(CLIENT_EXE): $(CFILES_CLIENT)
	@gcc $(CFLAGS) $(LDFLAGS) -o $(CLIENT_EXE) $(CFILES_CLIENT)

graphics.exe: main.c graphics.c message_list.c
	gcc -lGL -lGLU -lglut -lm main.c graphics.c message_list.c

.PHONY: clean

clean: 
	rm $(SERVER_EXE) $(CLIENT_EXE)