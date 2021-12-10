#ifndef _PONG_H
#define _PONG_H

#define MAX_TEAM_COUNT                      2 /* max teams in a single match */
#define MAX_PLAYER_COUNT                    4 /* max players in a single match */
#define MAX_BALL_COUNT                      1
#define MAX_POWERUP_COUNT                   3

#define MAX_NAME_SIZE                       20
#define MAX_MESSAGE_SIZE                    256
#define STATUS_SIZE                         1 
#define INPUT_SIZE                          1
#define GAME_STATISTICS_SIZE                112 // ??
#define GAMEBOARD_STATE_SIZE                176 // ??
#define GAME_STATE_SIZE                     GAMEBOARD_STATE_SIZE + 0 /* ?????? */

typedef struct _game_lobby {
    char player_id[MAX_PLAYER_COUNT];
} game_lobby;

typedef struct _game_state {
    char player_test_id;
} game_state;

#endif