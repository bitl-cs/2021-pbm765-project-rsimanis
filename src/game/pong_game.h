#ifndef _PONG_GAME_H
#define _PONG_GAME_H

#include "../utils/pong_math.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "../utils/debug.h"

/* gameloop */
#define GAME_READY_UPDATE_INTERVAL          1/30.0
#define GAME_STATE_UPDATE_INTERVAL          1/100.0      /* time interval (in seconds) between two game state updates */
#define GAME_RESTART_WAIT_TIME              0

/* general */
#define MIN_NAME_LENGTH                     3
#define MAX_NAME_LENGTH                     19
#define MAX_MESSAGE_LENGTH                  255         /* max message size that can be sent (255 chars + 1 null byte) */
#define GAME_TYPE_1V1                       1
#define GAME_TYPE_2V2                       2

/* window */
#define GAME_WINDOW_WIDTH                   800         /* game screen width (in pixels) */
#define GAME_WINDOW_HEIGHT                  600         /* game screen height (in pixels) */

/* game state */
#define GAME_STATE_STATUS_FREE                     -5          /* game state memory is not occupied */
#define GAME_STATE_STATUS_TAKEN                    -4          /* game state memory is occupied */
#define GAME_STATE_STATUS_LOADING                  -3          /* clients are currently loading the game screen */
#define GAME_STATE_STATUS_IN_PROGRESS              -2          /* the game is happening right now */
#define GAME_STATE_STATUS_STATISTICS               -1           /* players are notified with statistics, and the server is waiting when they will exit statistics screen */
#define GAME_STATE_STATUS_SUCCESS                   0           /* the game has finished with an error */
#define GAME_STATE_STATUS_CLIENT_FAILED_TO_LOAD     1           /* the game has finished without an error */
#define GAME_STATE_STATUS_ALL_TEAM_LEFT             2
#define GAME_STATE_STATUS_CLIENT_ERROR              3

/* teams */
#define MAX_TEAM_COUNT                      2           /* max teams in a single match */
#define TEAM_INITIAL_COUNT                  MAX_TEAM_COUNT
#define TEAM_INITIAL_SCORE                  0
#define TEAM_WINNING_SCORE_THRESHOLD        11
#define TEAM_WINNING_SCORE_DIFFERENCE       7
#define LEFT_TEAM_ID                        0
#define RIGHT_TEAM_ID                       1
#define LEFT_GOAL_LINE_UPPER_X              0
#define LEFT_GOAL_LINE_UPPER_Y              0
#define LEFT_GOAL_LINE_BOTTOM_X             0
#define LEFT_GOAL_LINE_BOTTOM_Y             GAME_WINDOW_HEIGHT 
#define RIGHT_GOAL_LINE_UPPER_X             GAME_WINDOW_WIDTH 
#define RIGHT_GOAL_LINE_UPPER_Y             0
#define RIGHT_GOAL_LINE_BOTTOM_X            GAME_WINDOW_WIDTH 
#define RIGHT_GOAL_LINE_BOTTOM_Y            GAME_WINDOW_HEIGHT 

/* players */
#define MAX_PLAYER_COUNT                    4           /* max players in a single match */
#define GAME_1V1_PLAYER_COUNT               2
#define GAME_2V2_PLAYER_COUNT               4
#define PLAYER_INITIAL_WIDTH                20
#define PLAYER_INITIAL_HEIGHT               100
#define PLAYER_INITIAL_SCORE                0
#define PLAYER_DISCONNECTED_CLIENT_ID       -1
#define LEFT_BACK_PLAYER_ID                 0
#define RIGHT_BACK_PLAYER_ID                1
#define LEFT_FRONT_PLAYER_ID                2
#define RIGHT_FRONT_PLAYER_ID               3
#define PLAYER_LEFT                         1
#define PLAYER_RIGHT                        0
#define BACK_PLAYER_DIST_FROM_GOAL_LINE     50
#define FRONT_PLAYER_DIST_FROM_BACK_PLAYER  80
#define FRONT_PLAYER_DIST_FROM_GOAL_LINE    (BACK_PLAYER_DIST_FROM_GOAL_LINE + FRONT_PLAYER_DIST_FROM_BACK_PLAYER)
#define LEFT_FRONT_PLAYER_INITIAL_X         FRONT_PLAYER_DIST_FROM_GOAL_LINE
#define LEFT_FRONT_PLAYER_INITIAL_Y         50
#define RIGHT_FRONT_PLAYER_INITIAL_X        (GAME_WINDOW_WIDTH - FRONT_PLAYER_DIST_FROM_GOAL_LINE - PLAYER_INITIAL_WIDTH)
#define RIGHT_FRONT_PLAYER_INITIAL_Y        (GAME_WINDOW_HEIGHT - LEFT_FRONT_PLAYER_INITIAL_Y - PLAYER_INITIAL_HEIGHT)
#define LEFT_BACK_PLAYER_INITIAL_X          BACK_PLAYER_DIST_FROM_GOAL_LINE
#define LEFT_BACK_PLAYER_INITIAL_Y          (GAME_WINDOW_HEIGHT - LEFT_FRONT_PLAYER_INITIAL_Y - PLAYER_INITIAL_HEIGHT) 
#define RIGHT_BACK_PLAYER_INITIAL_X         (GAME_WINDOW_WIDTH - BACK_PLAYER_DIST_FROM_GOAL_LINE - PLAYER_INITIAL_WIDTH)
#define RIGHT_BACK_PLAYER_INITIAL_Y         LEFT_FRONT_PLAYER_INITIAL_Y
#define PLAYER_INITIAL_VELOCITY_X           0
#define PLAYER_INITIAL_VELOCITY_Y           0
#define PLAYER_MAX_VELOCITY_MOD             10
#define PLAYER_INITIAL_ACCELERATION_X       0
#define PLAYER_INITIAL_ACCELERATION_Y       0
#define PLAYER_ACCELERATION_MOD             0.2
#define PLAYER_FRICTION                     0.8
#define PLAYER_READY_TRUE                   1           /* player has finished loading the game screen */
#define PLAYER_READY_FALSE                  0           /* player has not yet finished loading the game screen */

/* balls */
#define MAX_BALL_COUNT                      1           /* max balls in a single match */
#define BALL_INITIAL_COUNT                  MAX_BALL_COUNT
#define BALL_COLLISION_DISTANCE             20
#define BALL_INITIAL_X                      GAME_WINDOW_WIDTH / 2.0
#define BALL_INITIAL_Y                      GAME_WINDOW_HEIGHT / 2.0
#define BALL_INITIAL_VELOCITY_MOD           5
#define BALL_MAX_VELOCITY_MOD               20 
#define BALL_INITIAL_ACCELERATION_X         0
#define BALL_INITIAL_ACCELERATION_Y         0
#define BALL_MAX_ACCELERATION               0.1 
#define BALL_INITIAL_RADIUS                 10
#define BALL_INITIAL_TYPE                   BALL_TYPE_NORMAL 
#define BALL_INITIAL_LAST_TOUCHED_ID        -1
#define BALL_TYPE_NORMAL                    0
#define BALL_TYPE_ACCELERATING              1
#define BALL_INIT_ANGLE_IN_DEGREES          60

/* power-ups */
#define MAX_POWER_UP_COUNT                  3           /* max power-ups in a single match */
#define POWER_UP_INITIAL_COUNT              0
#define POWER_UP_TYPE_ACCELERATION          0
#define POWER_UP_TYPE_PADDLE_HEIGHT         1


typedef struct _team {
    char id;
    int score;
    int active_player_count;
    vec2f goal1;            /* upper point of goal-line */
    vec2f goal2;            /* bottom point of goal-line */
} team;

typedef struct _player {
    char id;                /* player's game id (not global) - equal to player index in players[] array in game_state */
    char client_id;         /* player's client id (global) - equal to client's index in clients[] array in server_shared_memory */
    char team_id;           /* only during the game (not global) - equal to team index in teams[] array in game_state*/
    char ready;             /* indicates whether player has finished loading game screen */
    char *name;             /* players's name */
    int score;              /* times player have scored */
    vec2f pos;              /* position */
    vec2f v;                /* velocity */
    vec2f a;                /* acceleration */
    char should_decelerate;
    float width, height;    /* paddle width and height */
} player;

typedef struct _ball {
    vec2f pos;
    vec2f v;
    vec2f a;
    float radius;
    char type;
    char last_touched_id;
} ball;

typedef struct _power_up {
    vec2f pos;
    float width, height;
    char type;
} power_up;

typedef struct _game_state {
    clock_t last_update;
    clock_t start_time;
    clock_t end_time;
    char status; 
    char game_type;
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
} game_state;


/* gameloop */
void start_game(game_state *gs);
void update_game_state(game_state *gs);
void restart_round(game_state *gs);
void end_game(game_state *gs, int status);

/* game_state */
void init_screen(game_state *gs);
void print_game_state(game_state *gs);

/* teams */
void init_team(team *team, char id, int active_player_count, float goal_x1, float goal_y1, float goal_x2, float goal_y2);
int is_winning_team(team *team, game_state *gs);
void print_team(team *team);

/* players */
void init_player(player *player, char id, char client_id, char team_id, char *name, float x, float y);
void update_player(player *player);
int all_players_ready(game_state *gs);
int all_players_disconnected(game_state *gs);
void reset_back_players(game_state *gs);
void reset_front_players(game_state *gs);
void print_player(player *player);

/* balls */
void init_balls(game_state *gs);
void init_ball_velocity(ball *ball);
void update_ball(ball *player, game_state *gs);
int is_colliding(ball *ball, player *player);
void print_ball(ball *ball);

/* power-ups */
void init_power_up(power_up *power_up, float x, float y, float width, float height, char type);
void reset_power_ups(game_state *gs);
void print_power_up(power_up *power_up);

/* helpers */
void update_velocity(vec2f *v, vec2f *a, float max_v_mod);
double time_diff_in_seconds(clock_t t1, clock_t t2);

#endif