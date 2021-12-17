OUT = lib/libgraphics.a
CC = gcc
ODIR = obj
SDIR = src/graphics
INC = -Iinc

_OBJS = a_pong_graphics_game.o \
		a_pong_graphics_general.o \
		a_pong_graphics_join.o \
		a_pong_graphics_lobby.o \
		a_pong_graphics_menu.o \
		a_pong_graphics_statistics.o \
		a_pong_graphics.o \
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))


$(ODIR)/%.o: $(SDIR)/%.c 
    $(CC) -c $(INC) -o $@ $< $(CFLAGS) 

$(OUT): $(OBJS) 
    ar rvs $(OUT) $^

.PHONY: clean

clean:
    rm -f $(ODIR)/*.o $(OUT)