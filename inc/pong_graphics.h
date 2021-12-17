#ifndef _GRAPHICS_PONG_
#define _GRAPHICS_PONG_

#include "pong_graphics_join.h"
#include "pong_graphics_menu.h"
#include "pong_graphics_lobby.h"
#include "pong_graphics_game.h"
#include "pong_graphics_statistics.h"

#include "../utils/message_list.h"
#include "../utils/message_list.h"
#include "../networking/pong_networking.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>


/* States - TO CLIENT ? */
#define CLIENT_STATE_JOIN                                           0
#define CLIENT_STATE_MENU                                           1
#define CLIENT_STATE_LOBBY                                          2
#define CLIENT_STATE_GAME                                           3
#define CLIENT_STATE_STATISTICS                                     4

/* Keyboard Listener - TO CLIENT */
#define CLIENT_GAMELOOP_UPDATE_INTERVAL                             1/20.0


/* Message List  - TO CLIENT ?*/
#define MAX_MESSAGE_LIST_SIZE                                       5

void render();
void gameloop();

#endif