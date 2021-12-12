#include "pong_game.h"

#include <stdlib.h>

void update_game_state(game_state *gs) {
    double diff;
    clock_t now;

    now = clock();
    diff = (double) (now - gs->last_update) / CLOCKS_PER_SEC;
    if (diff >= GAMELOOP_UPDATE_INTERVAL) {
        gs->last_update = now;
        
        // TODO: update
    }
}

/* helpers */
player *find_player_by_id(char player_id, game_state *gs) {
    char i;

    for (i = 0; i < gs->player_count; i++) {
        if (gs->players[i].id == player_id)
            return &gs->players[i];
    }
    return NULL;
}