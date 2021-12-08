#ifndef _PONG_H
#define _PONG_H

#define MAX_TEAM_COUNT                      2
#define MAX_PLAYER_COUNT                    4 /* max players in single match */
#define MAX_BALL_COUNT                      1
#define MAX_POWERUP_COUNT                   3

#define MAX_NAME_SIZE                       20
#define MAX_MESSAGE_SIZE                    256
#define STATUS_SIZE                         1 
#define INPUT_SIZE                          1
#define GAME_STATISTICS_SIZE                112 // ??
#define GAMEBOARD_STATE_SIZE                176 // ??
#define GAME_STATE_SIZE                     GAMEBOARD_STATE_SIZE + 0 /* ?????? */

void process_join(char *data);
void process_accept(char *data);
void process_message(char *data);
void process_lobby(char *data);
void process_game_ready(char *data);
void process_player_ready(void);
void process_game_state(char *data);
void process_player_input(char *data);
void process_check_status(void);
void process_game_end(char *data);

#endif