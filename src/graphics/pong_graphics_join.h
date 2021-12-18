#ifndef _PONG_GRAPHICS_JOIN_H
#define _PONG_GRAPHICS_JOIN_H

#include "pong_graphics.h"

#define JOIN_BUTTON_WIDTH                                   80
#define JOIN_BUTTON_HEIGHT                                  30
#define JOIN_BUTTON_X                                       ((WINDOW_WIDTH - JOIN_BUTTON_WIDTH) / 2)
#define JOIN_BUTTON_Y                                       ((WINDOW_HEIGHT - JOIN_BUTTON_HEIGHT) / 2)
#define JOIN_BUTTON_TEXT                                    "Join"

#define JOIN_NAME_FIELD_X                                   ((WINDOW_WIDTH - JOIN_NAME_FIELD_WIDTH) / 2)
#define JOIN_NAME_FIELD_Y                                   (JOIN_BUTTON_Y - JOIN_NAME_FIELD_DISTANCE_FROM_BELOW)
#define JOIN_NAME_FIELD_WIDTH                               215 
#define JOIN_NAME_FIELD_HEIGHT                              40
#define JOIN_NAME_FIELD_DISTANCE_FROM_BELOW                 (JOIN_NAME_FIELD_HEIGHT + 10)
#define JOIN_NAME_FIELD_OUTLINE_COLOR                       RGB_YELLOW
#define JOIN_NAME_FIELD_TEXT_COLOR                          RGB_WHITE
#define JOIN_NAME_FIELD_MAX_LEN                             19

void render_join();
void join_button_listener(int button, int event, int x, int y);


#endif