#ifndef _PONG_GRAPHICS_MENU_H
#define _PONG_GRAPHICS_MENU_H

#include "pong_graphics.h"

/* Title */
#define MENU_TITLE_TEXT                                     "Choose your game type:"
#define MENU_TITLE_Y                                        (WINDOW_HEIGHT / 3)

/* For Both Buttons */
#define GAME_TYPE_BUTON_WIDTH                               160
#define GAME_TYPE_BUTON_HEIGHT                              60
#define DISTANCE_BETWEEN_GAME_TYPE_BUTTONS                  25
#define DISTANCE_FROM_MENU_TEXT                             80

/* 1V1 Button */
#define ONE_VS_ONE_BUTTON_X                                 (WINDOW_WIDTH / 2 - GAME_TYPE_BUTON_WIDTH - DISTANCE_BETWEEN_GAME_TYPE_BUTTONS)
#define ONE_VS_ONE_BUTTON_Y                                 (WINDOW_HEIGHT / 2 + DISTANCE_FROM_MENU_TEXT - 150)
#define ONE_VS_ONE_BUTTON_TEXT                              "1V1"

/* 2V2 Button */
#define TWO_VS_TWO_BUTTON_X                                 (WINDOW_WIDTH / 2 + DISTANCE_BETWEEN_GAME_TYPE_BUTTONS)
#define TWO_VS_TWO_BUTTON_Y                                 (WINDOW_HEIGHT / 2 + DISTANCE_FROM_MENU_TEXT - 150)
#define TWO_VS_TWO_BUTTON_TEXT                              "2v2"

void render_menu();
void menu_button_listener(int button, int event, int x, int y);

#endif
