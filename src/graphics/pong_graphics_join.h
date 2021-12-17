#ifndef _PONG_GRAPHICS_JOIN_H
#define _PONG_GRAPHICS_JOIN_H

#include "pong_graphics_general.h"

#define JOIN_BUTTON_WIDTH                                   80
#define JOIN_BUTTON_HEIGHT                                  30
#define JOIN_BUTTON_X                                       (WINDOW_WIDTH / 2 - JOIN_BUTTON_WIDTH / 2)
#define JOIN_BUTTON_Y                                       (WINDOW_HEIGHT / 2 - JOIN_BUTTON_HEIGHT / 2)
#define JOIN_BUTTON_TEXT                                    "Join"

#define JOIN_NAME_FIELD_OUTLINE_COLOR                       RGB_YELLOW
#define JOIN_NAME_FIELD_TEXT_COLOR                          RGB_WHITE

void render_join();
void join_button_listener(int button, int event, int x, int y);


#endif