#include "pong_game.h"
#include "pong_math.h"
#include "pong_server.h"

#include <stdio.h>
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

void init_player(player *player, char id, char client_id, char team_id, char ready, char *name, int score, float x, float y, float vx, float vy, float ax, float ay, float width, float height) {
    player->id = id;
    player->client_id = client_id;
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

void init_ball(ball *ball, float x, float y, float vx, float vy, float ax, float ay, float radius, char type, char last_touched_id) {
    ball->pos.x = x;
    ball->pos.y = y;
    ball->v.x = vx;
    ball->v.y = vy;
    ball->a.x = ax;
    ball->a.y = ay;
    ball->radius = radius;
    ball->type = type;
    ball->last_touched_id = last_touched_id;
}

void init_power_up(power_up *power_up, float x, float y, float width, float height, char type) {
    power_up->pos.x = x;
    power_up->pos.y = y;
    power_up->width = width;
    power_up->height = height;
    power_up->type = type;
}

void init_window(game_state *gs) {
    gs->window_width = WINDOW_WIDTH;
    gs->window_height = WINDOW_HEIGHT;
}

void start_game(game_state *gs) {
    gs->status = GAME_STATE_STATUS_IN_PROGRESS;
}

/* gameloop */
int should_update_game_state(game_state *gs) {
    clock_t now;
    double diff;

    now = clock();
    diff = (double) (now - gs->last_update) / CLOCKS_PER_SEC;
    return diff >= GAME_STATE_UPDATE_INTERVAL;
}

void update_game_state(game_state *gs) {
    char i, j;
    float new_pos_x, new_pos_y;
    team *t;
    player *p;
    ball *b;
    power_up *pu;

    /* iterate over all players */
    for (i = 0; i < gs->player_count; i++) {
        p = &gs->players[i];

        /* update velocity */
        update_velocity_component(&p->v.x, &p->a.x, PLAYER_MAX_VELOCITY_X_MOD);
        update_velocity_component(&p->v.y, &p->a.y, PLAYER_MAX_VELOCITY_Y_MOD);

        /* update positions */            
        p->pos.x += p->v.x;
        p->pos.y += p->v.y;

    }

    /* itereate over all balls */
    for (i = 0; i < gs->ball_count; i++) {
        b = &gs->balls[i];  

        /* update positions */            
        update_velocity_component(&b->v.x, &b->a.x, BALL_MAX_VELOCITY_X_MOD);
        update_velocity_component(&b->v.y, &b->a.y, BALL_MAX_VELOCITY_Y_MOD);

        /* update positions */            
        b->pos.x += b->v.x;
        b->pos.y += b->v.y;

        /* check for colision with goal lines */
        // iterate over all teams
            // if colliding with line
                // update player score, team score, reset game board
        for (j = 0; j < gs->team_count; j++) {
            t = &gs->teams[j];
            // assuming that goal lines are ALWAYS vertical
            if (mod_f(b->pos.x - t->goal1.x) < BALL_COLLISION_DISTANCE) {
                gs->players[b->last_touched_id].score += 1;
                gs->teams[gs->players[b->last_touched_id].team_id].score += 1;
                if (is_winning_team(&gs->teams[gs->players[b->last_touched_id].team_id], gs))
                    end_game(gs);
                else
                    restart_round(gs);
            }
        }

        /* check for collision with players */
        for (j = 0; j < gs->player_count; j++) {
            p = &gs->players[j];

            if ((b->pos.y >= p->pos.y) && (b->pos.y <= p->pos.y + p->height) && 
                (b->pos.x + b->radius >= p->pos.x) && (b->pos.x - b->radius <= p->pos.x + p->width)) {
                reverse_vec2f(&b->v);
                reverse_vec2f(&b->a);
            }
        }

        /* check for collision with walls (top and bottom) */
        if ((b->pos.y >= WINDOW_HEIGHT - b->radius) || (b->pos.y <= b->radius)) {
            b->a.y = -b->a.y;
            b->v.y = -b->v.y;
        }

        /* check for collision with power-ups */
        // iterate over all power-ups 
            // if ball is inside power-up
                // apply powerup to ball/player based on its type, and remove it from the map
    }

    gs->last_update = clock();
}

void end_game(game_state *gs) {
    gs->end_time = clock();
    gs->status = GAME_STATE_STATUS_SUCCESS;
}

void restart_round(game_state *gs) {
    char i;

    reset_back_players(gs);
    if (gs->game_type == GAME_TYPE_2V2)
        reset_front_players(gs);
    reset_balls(gs);
    reset_power_ups(gs);
}

void update_velocity_component(float *v, float *a, float max_v_mod) {
    if (diff_signs_f(*v, *a)) {
        if (mod_f(*v) <= max_v_mod)
            *v += *a; 
    }
    else
        *v += *a;
}

int is_winning_team(team *team, game_state *gs) {
    char i;
    struct _team *t;

    if (team->score >= TEAM_WINNING_SCORE_THRESHOLD)
        return 1;

    for (i = 0; i < gs->team_count; i++) {
        t = &gs->teams[i];
        if (t == team)
            continue;
        if (team->score - t->score >= TEAM_WINNING_SCORE_DIFFERENCE)
            return 1;
    } 
    return 0;
}

void reset_back_players(game_state *gs) {
    player *lp, *rp;

    lp = &gs->players[LEFT_BACK_PLAYER_ID];
    lp = &gs->players[LEFT_BACK_PLAYER_ID];
    lp->pos.x = LEFT_BACK_PLAYER_INITIAL_X;
    lp->pos.y = LEFT_BACK_PLAYER_INITIAL_Y;
    lp->v.x = PLAYER_INITIAL_VELOCITY_X;
    lp->v.y = PLAYER_INITIAL_VELOCITY_Y;
    lp->a.x = PLAYER_INITIAL_ACCELERATION_X;
    lp->a.y = PLAYER_INITIAL_ACCELERATION_Y;
    lp->width = PLAYER_INITIAL_WIDTH;
    lp->height = PLAYER_INITIAL_HEIGHT;

    rp = &gs->players[RIGHT_BACK_PLAYER_ID];
    rp->pos.x = RIGHT_BACK_PLAYER_INITIAL_X;
    rp->pos.y = RIGHT_BACK_PLAYER_INITIAL_Y;
    rp->v.x = PLAYER_INITIAL_VELOCITY_X;
    rp->v.y = PLAYER_INITIAL_VELOCITY_Y;
    rp->a.x = PLAYER_INITIAL_ACCELERATION_X;
    rp->a.y = PLAYER_INITIAL_ACCELERATION_Y;
    rp->width = PLAYER_INITIAL_WIDTH;
    rp->height = PLAYER_INITIAL_HEIGHT;

}

void reset_front_players(game_state *gs) {
    player *lp, *rp;

    lp = &gs->players[LEFT_FRONT_PLAYER_ID];
    lp->pos.x = LEFT_FRONT_PLAYER_INITIAL_X;
    lp->pos.y = LEFT_FRONT_PLAYER_INITIAL_Y;
    lp->v.x = PLAYER_INITIAL_VELOCITY_X;
    lp->v.y = PLAYER_INITIAL_VELOCITY_Y;
    lp->a.x = PLAYER_INITIAL_ACCELERATION_X;
    lp->a.y = PLAYER_INITIAL_ACCELERATION_Y;
    lp->width = PLAYER_INITIAL_WIDTH;
    lp->height = PLAYER_INITIAL_HEIGHT;

    rp = &gs->players[RIGHT_FRONT_PLAYER_ID];
    rp->pos.x = RIGHT_FRONT_PLAYER_INITIAL_X;
    rp->pos.y = RIGHT_FRONT_PLAYER_INITIAL_Y;
    rp->v.x = PLAYER_INITIAL_VELOCITY_X;
    rp->v.y = PLAYER_INITIAL_VELOCITY_Y;
    rp->a.x = PLAYER_INITIAL_ACCELERATION_X;
    rp->a.y = PLAYER_INITIAL_ACCELERATION_Y;
    rp->width = PLAYER_INITIAL_WIDTH;
    rp->height = PLAYER_INITIAL_HEIGHT;

}

void reset_balls(game_state *gs) {
    char i;
    ball *b;
    
    for (i = 0; i < gs->ball_count; i++) {
        b = &gs->balls[i];
        b->pos.x = BALL_INITIAL_X;
        b->pos.y = BALL_INITIAL_Y;
        b->v.x = BALL_INITIAL_VELOCITY_X;
        b->v.y = BALL_INITIAL_VELOCITY_Y;
        b->a.x = BALL_INITIAL_ACCELERATION_X;
        b->a.y = BALL_INITIAL_ACCELERATION_Y;
        b->radius = BALL_INITIAL_RADIUS;
        b->last_touched_id = BALL_INITIAL_LAST_TOUCHED_ID;
        b->type = BALL_TYPE_NORMAL;
    }
}

void reset_power_ups(game_state *gs) {
    char i;
    power_up *pu;

    for (i = 0; i < gs->power_up_count; i++) {
        pu = &gs->power_ups[i];
        // reset
    }
}

int is_everyone_ready(game_state *gs) {
    char i;
    player *p;

    for (i = 0; i < gs->player_count; i++) {
        p = &gs->players[i];
        if (p->ready == PLAYER_READY_FALSE)
            return 0;
    }
    return 1;
}

void print_team(team *team) {
    printf("id: %d\n", team->id);
    printf("score: %d\n", team->score);
    printf("goal1: (%f, %f)\n", team->goal1.x, team->goal1.y);
    printf("goal2: (%f, %f)\n", team->goal2.x, team->goal2.y);
}

void print_player(player *player) {
    printf("id: %d\n", player->id);
    printf("client_id: %d\n", player->client_id);
    printf("team_id: %d\n", player->team_id);
    printf("ready: %d\n", player->ready);
    printf("name: %s\n", player->name);
    printf("score: %d\n", player->score);
    printf("pos: (%f, %f)\n", player->pos.x, player->pos.y);
    printf("v: (%f, %f)\n", player->v.x, player->v.y);
    printf("a: (%f, %f)\n", player->a.x, player->a.y);
    printf("width: %f\n", player->width);
    printf("height: %f\n", player->height);
}

void print_ball(ball *ball) {
    printf("pos: (%f, %f)\n", ball->pos.x, ball->pos.y);
    printf("v: (%f, %f)\n", ball->v.x, ball->v.y);
    printf("a: (%f, %f)\n", ball->a.x, ball->a.y);
    printf("radius: %f\n", ball->radius);
    printf("type: %d\n", ball->type);
    printf("last_touched_id: %d\n", ball->last_touched_id);
}

void print_power_up(power_up *power_up) {
    printf("pos: (%f, %f)\n", power_up->pos.x, power_up->pos.y);
    printf("width: %f\n", power_up->width);
    printf("height: %f\n", power_up->height);
    printf("type: %d\n", power_up->type);
}

void print_game_state(game_state *gs) {
    char i;

    printf("--GENERAL INFO--\n");
    printf("last_update: %lu\n", gs->last_update);
    printf("start_time: %lu\n", gs->start_time);
    printf("end_time: %lu\n", gs->end_time);
    printf("status: %d\n", gs->status);
    printf("game_type: %d\n", gs->game_type);
    printf("window_width: %d\n", gs->window_width);
    printf("window_height: %d\n", gs->window_height);

    putchar('\n');
    printf("--TEAMS--\n");
    printf("team_count: %d\n", gs->team_count);
    putchar('\n');
    for (i = 0; i < gs->team_count; i++) {
        print_team(&gs->teams[i]);
        putchar('\n');
    }

    putchar('\n');
    printf("--PLAYERS--\n");
    printf("player_count: %d\n", gs->player_count);
    putchar('\n');
    for (i = 0; i < gs->player_count; i++) {
        print_player(&gs->players[i]);
        putchar('\n');
    }

    putchar('\n');
    printf("--BALLS--\n");
    printf("ball_count: %d\n", gs->ball_count);
    putchar('\n');
    for (i = 0; i < gs->ball_count; i++) {
        print_ball(&gs->balls[i]);
        putchar('\n');
    }

    putchar('\n');
    printf("--POWER_UPS--\n");
    putchar('\n');
    for (i = 0; i < gs->power_up_count; i++) {
        print_power_up(&gs->power_ups[i]);
        putchar('\n');
    }
}