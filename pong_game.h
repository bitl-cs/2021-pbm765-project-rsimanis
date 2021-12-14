#ifndef _PONG_GAME_H
#define _PONG_GAME_H

#include "pong_math.h"
#include <time.h>

/* gameloop */
#define GAME_READY_UPDATE_INTERVAL          1/30.0
#define GAME_STATE_UPDATE_INTERVAL          1/70.0      /* time interval (in seconds) between two game state updates */

/* general */
#define MAX_NAME_SIZE                       20          /* max client username size (19 chars + 1 null byte) */
#define MAX_MESSAGE_SIZE                    256         /* max message size that can be sent (255 chars + 1 null byte) */

#define GAME_STATE_STATUS_FREE              -4          /* game state memory is not occupied */
#define GAME_STATE_STATUS_TAKEN             -3          /* game state memory is occupied */
#define GAME_STATE_STATUS_IN_PROGRESS       -2          /* game state memory is occupied and the game is happening right now */
#define GAME_STATE_STATUS_LOADING           -1          /* game state memory is occupied and clients are currently loading the game screen */
#define GAME_STATE_STATUS_ERROR             0           /* game state memory is occupied and the game has finished with an error */
#define GAME_STATE_STATUS_SUCCESS           1           /* game state memory is occupied and the game has finished without an error */

#define GAME_TYPE_1V1                       1
#define GAME_TYPE_2V2                       2

/* window */
#define WINDOW_WIDTH                        800         /* game screen width (in pixels) */
#define WINDOW_HEIGHT                       600         /* game screen height (in pixels) */

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
#define LEFT_GOAL_LINE_BOTTOM_Y             WINDOW_HEIGHT 

#define RIGHT_GOAL_LINE_UPPER_X             WINDOW_WIDTH 
#define RIGHT_GOAL_LINE_UPPER_Y             0
#define RIGHT_GOAL_LINE_BOTTOM_X            WINDOW_WIDTH 
#define RIGHT_GOAL_LINE_BOTTOM_Y            WINDOW_HEIGHT 

/* players */
#define MAX_PLAYER_COUNT                    4           /* max players in a single match */
#define GAME_1V1_PLAYER_COUNT               2
#define GAME_2V2_PLAYER_COUNT               4

#define PLAYER_INITIAL_WIDTH                50
#define PLAYER_INITIAL_HEIGHT               100
        
#define PLAYER_INITIAL_SCORE                0

#define LEFT_BACK_PLAYER_ID                 0
#define RIGHT_BACK_PLAYER_ID                1
#define LEFT_FRONT_PLAYER_ID                2
#define RIGHT_FRONT_PLAYER_ID               3

#define PLAYER_LEFT                         1
#define PLAYER_RIGHT                        0

#define FRONT_PLAYER_DIST_FROM_GOAL_LINE    250
#define BACK_PLAYER_DIST_FROM_GOAL_LINE     100

#define LEFT_FRONT_PLAYER_INITIAL_X         FRONT_PLAYER_DIST_FROM_GOAL_LINE
#define LEFT_FRONT_PLAYER_INITIAL_Y         50
#define RIGHT_FRONT_PLAYER_INITIAL_X        (WINDOW_WIDTH - FRONT_PLAYER_DIST_FROM_GOAL_LINE)
#define RIGHT_FRONT_PLAYER_INITIAL_Y        (WINDOW_HEIGHT - LEFT_FRONT_PLAYER_INITIAL_Y)

#define LEFT_BACK_PLAYER_INITIAL_X          BACK_PLAYER_DIST_FROM_GOAL_LINE
#define LEFT_BACK_PLAYER_INITIAL_Y          (WINDOW_HEIGHT - LEFT_FRONT_PLAYER_INITIAL_Y) 
#define RIGHT_BACK_PLAYER_INITIAL_X         (WINDOW_WIDTH - FRONT_PLAYER_DIST_FROM_GOAL_LINE)
#define RIGHT_BACK_PLAYER_INITIAL_Y         LEFT_FRONT_PLAYER_INITIAL_Y

#define PLAYER_INITIAL_VELOCITY_X           0
#define PLAYER_INITIAL_VELOCITY_Y           0
#define PLAYER_MAX_VELOCITY_X_MOD           0
#define PLAYER_MAX_VELOCITY_Y_MOD           25

#define PLAYER_INITIAL_ACCELERATION_X       0
#define PLAYER_INITIAL_ACCELERATION_Y       0
#define PLAYER_ACCELERATION_X_MOD           0           /* magnitude of acceleration */
#define PLAYER_ACCELERATION_Y_MOD           5           /* magnitude of acceleration */


#define PLAYER_READY_TRUE                   1           /* player has finished loading the game screen */
#define PLAYER_READY_FALSE                  0           /* player has not yet finished loading the game screen */

/* balls */
#define MAX_BALL_COUNT                      1           /* max balls in a single match */
#define BALL_INITIAL_COUNT                  MAX_BALL_COUNT

#define BALL_COLLISION_DISTANCE             5

#define BALL_INITIAL_X                      WINDOW_WIDTH / 2.0
#define BALL_INITIAL_Y                      WINDOW_HEIGHT / 2.0

#define BALL_INITIAL_VELOCITY_X             5
#define BALL_INITIAL_VELOCITY_Y             -2
#define BALL_MAX_VELOCITY_X_MOD             50
#define BALL_MAX_VELOCITY_Y_MOD             50

#define BALL_INITIAL_ACCELERATION_X         0
#define BALL_INITIAL_ACCELERATION_Y         0
#define BALL_ACCELERATION_X_MOD             5
#define BALL_ACCELERATION_Y_MOD             5

#define BALL_INITIAL_RADIUS                 10
#define BALL_INITIAL_TYPE                   BALL_TYPE_NORMAL 
#define BALL_INITIAL_LAST_TOUCHED_ID        -1

#define BALL_TYPE_NORMAL                    0
#define BALL_TYPE_ACCELERATING              1

/* power-ups */
#define MAX_POWER_UP_COUNT                  3           /* max power-ups in a single match */
#define POWER_UP_INITIAL_COUNT              0

#define POWER_UP_TYPE_ACCELERATION          0
#define POWER_UP_TYPE_PADDLE_HEIGHT         1


typedef struct _team {
    char id;
    int score;
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

/* init */
void init_window(game_state *gs);
void init_team(team *team, char id, int score, float goal_x1, float goal_y1, float goal_x2, float goal_y2);
void init_player(player *player, char id, char client_id, char team_id, char ready, char *name, int score, float x, float y, float vx, float vy, float ax, float ay, float width, float height);
void init_ball(ball *ball, float x, float y, float vx, float vy, float ax, float ay, float radius, char type, char last_touched_id);
void init_power_up(power_up *power_up, float x, float y, float width, float height, char type);
void start_game(game_state *gs);

/* gameloop */
int should_update_game_state(game_state *gs);
void update_game_state(game_state *gs);
void restart_round(game_state *gs);
void end_game(game_state *gs);
int is_everyone_ready(game_state *gs);

/* helpers */
// player *find_player_by_id(char player_id, game_state *gs);
int is_colliding(ball *ball, player *player);
void update_velocity_component(float *v, float *a, float max_v_mod);
int is_winning_team(team *team, game_state *gs);
void reset_back_players(game_state *gs);
void reset_front_players(game_state *gs);
void reset_balls(game_state *gs);
void reset_power_ups(game_state *gs);

/* debug */
void print_team(team *team);
void print_player(player *player);
void print_ball(ball *ball);
void print_power_up(power_up *power_up);
void print_game_state(game_state *gs);

#endif