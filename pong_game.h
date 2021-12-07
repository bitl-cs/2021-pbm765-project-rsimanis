#ifndef _PONG_H
#define _PONG_H

#define MAX_NAME_SIZE                       20
#define INPUT_SIZE                 1
#define GAME_STATISTICS_SIZE                112
#define GAMEBOARD_STATE_SIZE                176
#define GAME_STATE_SIZE                     GAMEBOARD_STATE_SIZE + 0 /* ?????? */

typedef struct _game_state {
   int test; 
} game_state;

typedef struct _game_statistics {
    int test;
} game_statistics;

#endif