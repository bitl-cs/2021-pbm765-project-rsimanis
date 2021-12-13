#ifndef _PONG_GAME_H
#define _PONG_GAME_H

#include "pong_math.h"
#include <time.h>

/* gameloop */
#define GAMELOOP_UPDATE_INTERVAL            1/70.0      /* time interval (in seconds) between two game state updates */

/* general */
#define MAX_NAME_SIZE                       20          /* max client username size (19 chars + 1 null byte) */
#define MAX_MESSAGE_SIZE                    256         /* max message size that can be sent (255 chars + 1 null byte) */

#define GAME_STATE_STATUS_FREE              -2          /* game state memory is not occupied */
#define GAME_STATE_STATUS_IN_PROGRESS       -1          /* game state memory is occupied and the game is happening right now */
#define GAME_STATE_STATUS_ERROR             0           /* game state memory is occupied and the game has finished with an error */
#define GAME_STATE_STATUS_SUCCESS           1           /* game state memory is occupied and the game has finished without an error */

/* screen */
#define SCREEN_WIDTH                        800         /* game screen width (in pixels) */
#define SCREEN_HEIGHT                       600         /* game screen height (in pixels) */

/* teams */
#define MAX_TEAM_COUNT                      2           /* max teams in a single match */
#define INITIAL_TEAM_COUNT                  MAX_TEAM_COUNT

#define INITIAL_TEAM_SCORE                  0

#define LEFT_TEAM_ID                        0
#define RIGHT_TEAM_ID                       1

#define LEFT_GOAL_LINE_UPPER_X              0
#define LEFT_GOAL_LINE_UPPER_Y              0
#define LEFT_GOAL_LINE_BOTTOM_X             0
#define LEFT_GOAL_LINE_BOTTOM_Y             SCREEN_HEIGHT 

#define RIGHT_GOAL_LINE_UPPER_X             SCREEN_WIDTH 
#define RIGHT_GOAL_LINE_UPPER_Y             0
#define RIGHT_GOAL_LINE_BOTTOM_X            SCREEN_WIDTH 
#define RIGHT_GOAL_LINE_BOTTOM_Y            SCREEN_HEIGHT 

/* players */
#define MAX_PLAYER_COUNT                    4           /* max players in a single match */
#define GAME_1V1_PLAYER_COUNT               2
#define GAME_2V2_PLAYER_COUNT               4

#define INITIAL_PLAYER_WIDTH                50
#define INITIAL_PLAYER_HEIGHT               100

#define INITIAL_PLAYER_SCORE                0

#define BACK_LEFT_PLAYER_INDEX              0
#define FRONT_LEFT_PLAYER_INDEX             1
#define BACK_RIGHT_PLAYER_INDEX             2
#define FRONT_RIGHT_PLAYER_INDEX            3

#define FRONT_PLAYER_DIST_FROM_GOAL_LINE    250
#define BACK_PLAYER_DIST_FROM_GOAL_LINE     100

#define INITIAL_LEFT_FRONT_PLAYER_X         FRONT_PLAYER_DIST_FROM_GOAL_LINE
#define INITIAL_LEFT_FRONT_PLAYER_Y         50
#define INITIAL_RIGHT_FRONT_PLAYER_X        (SCREEN_WIDTH - FRONT_PLAYER_DIST_FROM_GOAL_LINE)
#define INITIAL_RIGHT_FRONT_PLAYER_Y        (SCREEN_HEIGHT - INITIAL_LEFT_FRONT_PLAYER_Y)

#define INITIAL_LEFT_BACK_PLAYER_X          BACK_PLAYER_DIST_FROM_GOAL_LINE
#define INITIAL_LEFT_BACK_PLAYER_Y          (SCREEN_HEIGHT - INITIAL_LEFT_FRONT_PLAYER_Y) 
#define INITIAL_RIGHT_BACK_PLAYER_X         (SCREEN_WIDTH - FRONT_PLAYER_DIST_FROM_GOAL_LINE)
#define INITIAL_RIGHT_BACK_PLAYER_Y         INITIAL_LEFT_FRONT_PLAYER_Y

#define INITIAL_PLAYER_VELOCITY_X           0
#define INITIAL_PLAYER_VELOCITY_Y           0
#define PLAYER_MAX_VELOCITY_X_MOD           0
#define PLAYER_MAX_VELOCITY_Y_MOD           25

#define INITIAL_PLAYER_ACCELERATION_X       0
#define INITIAL_PLAYER_ACCELERATION_Y       0
#define PLAYER_ACCELERATION_X_MOD           0           /* magnitude of acceleration */
#define PLAYER_ACCELERATION_Y_MOD           5           /* magnitude of acceleration */


#define PLAYER_READY_TRUE                   1           /* player has finished loading the game screen */
#define PLAYER_READY_FALSE                  0           /* player has not yet finished loading the game screen */

/* balls */
#define MAX_BALL_COUNT                      1           /* max balls in a single match */
#define INITIAL_BALL_COUNT                  MAX_BALL_COUNT

#define INITIAL_BALL_X                      SCREEN_WIDTH / 2.0
#define INITIAL_BALL_Y                      SCREEN_HEIGHT / 2.0

#define INITIAL_BALL_VELOCITY_X             5
#define INITIAL_BALL_VELOCITY_Y             -2
#define BALL_MAX_VELOCITY_X_MOD             50
#define BALL_MAX_VELOCITY_Y_MOD             50

#define INITIAL_BALL_ACCELERATION_X         0
#define INITIAL_BALL_ACCELERATION_Y         0
#define BALL_ACCELERATION_X_MOD             5
#define BALL_ACCELERATION_Y_MOD             5

#define INITIAL_BALL_RADIUS                 10
#define INITIAL_BALL_TYPE                   BALL_TYPE_NORMAL 

#define BALL_TYPE_NORMAL                    0
#define BALL_TYPE_ACCELERATING              1

/* power-ups */
#define MAX_POWER_UP_COUNT                  3           /* max power-ups in a single match */
#define INITIAL_POWER_UP_COUNT              0

#define POWER_UP_TYPE_ACCELERATION          0
#define POWER_UP_TYPE_PADDLE_HEIGHT         1


typedef struct _team {
    char id;
    int score;
    vec2f goal1;            /* upper point of goal-line */
    vec2f goal2;            /* bottom point of goal-line */
} team;

typedef struct _player {
    char id;                /* equal to client_id (global) */
    char team_id;             /* only during the game (not global) */
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
    char status; 
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
void init_player(player *player, char id, char team_id, char ready, char *name, int score, float x, float y, float vx, float vy, float ax, float ay, float width, float height);
void init_ball(ball *ball, float x, float y, float vx, float vy, float ax, float ay, float radius, char type);
void init_power_up(power_up *power_up, float x, float y, float width, float height, char type);

/* gameloop */
void update_game_state(game_state *gs);
int is_colliding(ball *ball, player *player);

/* helpers */
player *find_player_by_id(char player_id, game_state *gs);

#endif