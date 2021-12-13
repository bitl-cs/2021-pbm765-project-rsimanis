#include "pong_game.h"

#include <stdlib.h>

/* init */
void init_team(team *team, char id, int score, float goal_x1, float goal_y1, float goal_x2, float goal_y2) {
    team->id = id;
    team->score = score;
    team->goal1.x = goal_x1;
    team->goal1.y = goal_y1;
    team->goal2.x = goal_x2;
    team->goal2.y = goal_y2;
}

void init_player(player *player, char id, char team_id, char ready, char *name, int score, float x, float y, float vx, float vy, float ax, float ay, float width, float height) {
    player->id = id;
    player->team_id = team_id;
    player->ready = ready;
    player->name = name;
    player->score = score;
    player->pos.x = x;
    player->pos.y = y;
    player->v.x = vx;
    player->v.y = vy;
    player->a.x = ax;
    player->a.y = ay;
    player->width = width;
    player->height = height;
}

void init_ball(ball *ball, float x, float y, float vx, float vy, float ax, float ay, float radius, char type) {
    ball->pos.x = x;
    ball->pos.y = y;
    ball->v.x = vx;
    ball->v.y = vy;
    ball->a.x = ax;
    ball->a.y = ay;
    ball->radius = radius;
    ball->type = type;
}

void init_power_up(power_up *power_up, float x, float y, float width, float height, char type) {
    power_up->pos.x = x;
    power_up->pos.y = y;
    power_up->width = width;
    power_up->height = height;
    power_up->type = type;
}

/* gameloop */
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