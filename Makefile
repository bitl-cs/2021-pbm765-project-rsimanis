CC = gcc
ODIR = obj
SDIR = src
LDIR = lib
BDIR = bin

CFLAGS = -std=gnu90 -fno-common
LDFLAGS = -lGL -lGLU -lglut -lm -lpthread

_LIB_CLIENT_OBJS = pong_client.o
LIB_CLIENT_OBJS = $(patsubst %,$(ODIR)/client/%,$(_LIB_CLIENT_OBJS))
_LIB_GAME_OBJS = pong_game.o
LIB_GAME_OBJS = $(patsubst %,$(ODIR)/game/%,$(_LIB_GAME_OBJS))
_LIB_GRAPHICS_OBJS = pong_graphics_game.o pong_graphics_join.o pong_graphics_lobby.o pong_graphics_menu.o pong_graphics_statistics.o pong_graphics.o
LIB_GRAPHICS_OBJS = $(patsubst %,$(ODIR)/graphics/%,$(_LIB_GRAPHICS_OBJS))
_LIB_NETWORKING_OBJS = pong_networking.o
LIB_NETWORKING_OBJS = $(patsubst %,$(ODIR)/networking/%,$(_LIB_NETWORKING_OBJS))
_LIB_SERVER_OBJS = pong_server.o
LIB_SERVER_OBJS = $(patsubst %,$(ODIR)/server/%,$(_LIB_SERVER_OBJS))
_LIB_UTILS_OBJS = args.o message_list.o pong_math.o debug.o
LIB_UTILS_OBJS = $(patsubst %,$(ODIR)/utils/%,$(_LIB_UTILS_OBJS))

#all
all: client server graphics
game: client server

# client
CLIENT_OUT = $(BDIR)/client.exe
CLIENT_MAIN_OBJ = $(ODIR)/client/client.o
CLIENT_OBJS = $(LIB_GAME_OBJS) $(LIB_NETWORKING_OBJS) $(LIB_UTILS_OBJS) $(LIB_GRAPHICS_OBJS) $(LIB_CLIENT_OBJS) $(CLIENT_MAIN_OBJ)
client: $(CLIENT_OUT)
$(CLIENT_OUT): $(CLIENT_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# server
SERVER_OUT = $(BDIR)/server.exe
SERVER_MAIN_OBJ = $(ODIR)/server/server.o
SERVER_OBJS = $(LIB_SERVER_OBJS) $(LIB_GAME_OBJS) $(LIB_NETWORKING_OBJS) $(LIB_UTILS_OBJS) $(SERVER_MAIN_OBJ)
server: $(SERVER_OUT)
$(SERVER_OUT): $(SERVER_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# graphics
GRAPHICS_OUT = $(BDIR)/graphics.exe
GRAPHICS_MAIN_OBJ = $(ODIR)/graphics/test_graphics.o
GRAPHICS_OBJS = $(LIB_GRAPHICS_OBJS) $(LIB_NETWORKING_OBJS) $(LIB_UTILS_OBJS) $(GRAPHICS_MAIN_OBJ)
graphics: $(GRAPHICS_OUT)
$(GRAPHICS_OUT): $(GRAPHICS_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

# clean
.PHONY: clean
clean:
	rm -rf $(ODIR)/*/*.o $(CLIENT_OUT) $(SERVER_OUT) $(GRAPHICS_OUT)