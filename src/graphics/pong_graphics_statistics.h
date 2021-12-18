#ifndef _PONG_GRAPHICS_STATISTICS_H
#define _PONG_GRAPHICS_STATISTICS_H

#include "pong_graphics.h"

#define STATISTICS_ERROR_MSG_X                              200
#define STATISTICS_ERROR_MSG_Y                              (WINDOW_WIDTH / 3)
#define STATISTICS_GENERAL_X                                300
#define STATISTICS_GENERAL_INITIAL_Y                        100
#define STATISTICS_LINE_SPACING                             30
#define STATISTICS_LEFT_TEAM_X                              100
#define STATISTICS_LEFT_TEAM_INITIAL_Y                      (STATISTICS_GENERAL_INITIAL_Y + 4 * STATISTICS_LINE_SPACING)
#define STATISTICS_RIGHT_TEAM_X                             600
#define STATISTICS_RIGHT_TEAM_INITIAL_Y                     (STATISTICS_GENERAL_INITIAL_Y + 4 * STATISTICS_LINE_SPACING)
#define STATISTICS_LEFT_TEAM_COLOR                          RGB_GREEN
#define STATISTICS_RIGHT_TEAM_COLOR                         RGB_RED

void render_statistics();
void statistics_button_listener(int button, int event, int x, int y);

#endif
