#ifndef _PONG_GAME_H
#define _PONG_GAME_H

#include "pong_math.h"
#include <time.h>

#define GAMELOOP_UPDATE_INTERVAL            1/70.0      /* in seconds */

#define MAX_TEAM_COUNT                      2           /* max teams in a single match */
#define MAX_PLAYER_COUNT                    4           /* max players in a single match */
#define MAX_BALL_COUNT                      1           /* max balls in a single match */
#define MAX_POWER_UP_COUNT                  3           /* max power-ups in a single match */

#define MAX_NAME_SIZE                       20          /* max client username size (19 chars + 1 null byte) */
#define MAX_MESSAGE_SIZE                    256         /* max message size that can be sent (255 chars + 1 null byte) */

#define PLAYER_STATE_JOIN                   0           /* player sees the join screen */
#define PLAYER_STATE_MENU                   1           /* player sees the main menu (1v1 and 2v2 buttons) */
#define PLAYER_STATE_LOBBY                  2           /* player sees his/her lobby */
#define PLAYER_STATE_LOADING                3           /* player sees the game loading screen before match */
#define PLAYER_STATE_GAME                   4           /* player sees the game - he is playing pong (hopefully) */ 
#define PLAYER_STATE_STATISTICS             5           /* player sees the statistics screen (or error if game finished erroneously*/

#define PLAYER_ACCELERATION                 5           /* magnitude of acceleration */

#define PLAYER_READY_TRUE                   1           /* player has finished loading the game screen */
#define PLAYER_READY_FALSE                  0           /* player has not yet finished loading the game screen */

#define SCREEN_WIDTH                        800         /* game screen width (in pixels) */
#define SCREEN_HEIGHT                       600         /* game screen height (in pixels) */

#define GAME_STATE_STATUS_FREE              -2          /* game state memory is not occupied */
#define GAME_STATE_STATUS_IN_PROGRESS       -1          /* game state memory is occupied and the game is happening right now */
#define GAME_STATE_STATUS_ERROR             0           /* game state memory is occupied and the game has finished with an error */
#define GAME_STATE_STATUS_SUCCESS           1           /* game state memory is occupied and the game has finished without an error */

typedef struct _team {
    char id;
    int score;
    vec2f goal1;
    vec2f goal2;
} team;

typedef struct _player {
    char id;                /* equal to client_id (global) */
    team *team;             /* only during the game (not global) */
    char ready;             /* indicates whether player has finished loading game screen */
    char *name;             /* players's name */
    int score;              /* times player have scored */
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
    clock_t last_update;
    char status;            /* -1 = not occupied, 0 = game in progress, 1 = successful end (send statistics), 2 = erroroneous exit (send error) */
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

/* helpers */
player *find_player_by_id(char player_id, game_state *gs);

#endif