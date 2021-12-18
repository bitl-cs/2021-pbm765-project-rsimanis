#ifndef _PONG_GRAPHICS_LOBBY_H
#define _PONG_GRAPHICS_LOBBY_H

#include "pong_graphics.h"

#define LOBBY_NAME_FIELD_LINE_SPACING               55
#define LOBBY_NAME_FIELD_X                          30
#define LOBBY_NAME_FIELD_INITIAL_Y                  100
#define LOBBY_NAME_FIELD_TEXT_COLOR                 RGB_WHITE

void render_lobby();
void lobby_button_listener(int button, int event, int x, int y);

#endif
