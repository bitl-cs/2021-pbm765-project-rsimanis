#ifndef _PONG_GAME_H
#define _PONG_GAME_H

#include "pong_math.h"

#define MAX_TEAM_COUNT                      2 /* max teams in a single match */
#define MAX_PLAYER_COUNT                    4 /* max players in a single match */
#define MAX_BALL_COUNT                      1
#define MAX_POWER_UP_COUNT                  3

#define MAX_NAME_SIZE                       20
#define MAX_MESSAGE_SIZE                    256
#define STATUS_SIZE                         1 
#define INPUT_SIZE                          1

#define PLAYER_STATE_JOIN                   0
#define PLAYER_STATE_MENU                   1
#define PLAYER_STATE_LOBBY                  2
#define PLAYER_STATE_GAME                   3 
#define PLAYER_STATE_STATISTICS             4

#define SCREEN_WIDTH                        800
#define SCREEN_HEIGHT                       600

typedef struct _team {
    char id;
    int score;
    vec2f goal1;
    vec2f goal2;
} team;

typedef struct _player {
    char id;
    char team_id;
    char ready;
    char *name;
    int score;
    vec2f pos;              /* position */
    vec2f v;                /* velocity */
    vec2f a;                /* acceleration */
    float width, height;    /* paddle width and height */
} player;

typedef struct _ball {
    vec2f pos;
    vec2f v;
    vec2f a;
    float radius;
    char type;
} ball;

typedef struct _power_up {
    vec2f pos;
    float width, height;
    char type;
} power_up;

typedef struct _game_state {
    int window_width;
    int window_height;
    char team_count;
    team teams[MAX_TEAM_COUNT];
    char player_count;
    player players[MAX_PLAYER_COUNT];
    char ball_count;
    ball balls[MAX_BALL_COUNT];
    char power_up_count;
    power_up power_ups[MAX_POWER_UP_COUNT]; 
    int start_time, end_time;
    char end_status;
} game_state;

/* init */
void init_team(team *team, char id, int score, float goal_x1, float goal_y1, float goal_x2, float goal_y2);
void init_player(player *player, char id, char team_id, float x, float y, float vx, float vy, float ax, float ay, float width, float height);
void init_ball(ball *ball, float x, float y, float vx, float vy, float ax, float ay, float radius, char type);
void init_power_up(power_up *power_up, float x, float y, float width, float height, char type);

void init_game_1v1(game_state *gs);
void init_game_2v2(game_state *gs);

/* gameloop */
void update_game_state(game_state *gs);

#endif