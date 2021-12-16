#ifndef _GRAPHICS_PONG_
#define _GRAPHICS_PONG_

#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <GL/freeglut.h>
#include <string.h>
#include "message_list.h"

#define WINDOW_HEIGHT                                       800
#define WINDOW_WIDTH                                        800 

#define PLAY_BUTTON_WIDTH                                   80
#define PLAY_BUTTON_HEIGHT                                  30
#define PLAY_BUTTON_X                                       (WINDOW_WIDTH / 2 - PLAY_BUTTON_WIDTH / 2)
#define PLAY_BUTTON_Y                                       (WINDOW_HEIGHT / 2 - PLAY_BUTTON_HEIGHT / 2)

#define GAME_TYPE_BUTON_WIDTH                               160
#define GAME_TYPE_BUTON_HEIGHT                              60
#define HORIZONTAL_OFFESST_BETWEEN_GAME_TYPE_BUTTONS        25
#define VERTICAL_OFFSET_BETWEEN_GAME_TYPE_BUTTONS           80

/* 1V1 */
#define ONE_V_ONE_X                                         (WINDOW_WIDTH / 2 - GAME_TYPE_BUTON_WIDTH - HORIZONTAL_OFFESST_BETWEEN_GAME_TYPE_BUTTONS)
#define ONE_V_ONE_Y                                         (WINDOW_HEIGHT / 2 + VERTICAL_OFFSET_BETWEEN_GAME_TYPE_BUTTONS) - 150

/* 2V2 */
#define TWO_V_TWO_X                                         (WINDOW_WIDTH / 2 + HORIZONTAL_OFFESST_BETWEEN_GAME_TYPE_BUTTONS)
#define TWO_V_TWO_Y                                         (WINDOW_HEIGHT / 2 + VERTICAL_OFFSET_BETWEEN_GAME_TYPE_BUTTONS) - 150

/* Chat */
#define CHAT_WINDOW_X                                       CHAT_WINDOW_DISTANCE_FROM_SIDE                      
#define CHAT_WINDOW_Y                                       (CHAT_TYPE_FIELD_Y - CHAT_WINDOW_DISTANCE_FROM_TYPE_FIELD - CHAT_WINDOW_HEIGHT)
#define CHAT_WINDOW_WIDTH                                   (WINDOW_WIDTH - (2 * CHAT_WINDOW_DISTANCE_FROM_SIDE))
#define CHAT_WINDOW_HEIGHT                                  150
#define CHAT_WINDOW_DISTANCE_FROM_SIDE                      10
#define CHAT_WINDOW_DISTANCE_FROM_TYPE_FIELD                10

#define CHAT_TYPE_FIELD_X                                   CHAT_TYPE_FIELD_DISTANCE_FROM_SIDE
#define CHAT_TYPE_FIELD_Y                                   (WINDOW_HEIGHT - CHAT_TYPE_FIELD_HEIGHT - CHAT_TYPE_FIELD_DISTANCE_FROM_BOTTOM)
#define CHAT_TYPE_FIELD_WIDTH                               (WINDOW_WIDTH - CHAT_SEND_ALL_BUTTON_WIDTH - 3 * CHAT_SEND_ALL_BUTTON_PADDING)
#define CHAT_TYPE_FIELD_HEIGHT                              30
#define CHAT_TYPE_FIELD_DISTANCE_FROM_SIDE                  10
#define CHAT_TYPE_FIELD_DISTANCE_FROM_BOTTOM                10

#define CHAT_SEND_ALL_BUTTON_X                              (WINDOW_WIDTH - CHAT_SEND_ALL_BUTTON_WIDTH - CHAT_SEND_ALL_BUTTON_PADDING)
#define CHAT_SEND_ALL_BUTTON_Y                              CHAT_TYPE_FIELD_Y
#define CHAT_SEND_ALL_BUTTON_WIDTH                          140
#define CHAT_SEND_ALL_BUTTON_HEIGHT                         CHAT_TYPE_FIELD_HEIGHT
#define CHAT_SEND_ALL_BUTTON_PADDING                        10

#define CHAT_SEND_BUTTON_X                                  (LOBBY_NAME_X + 200)
#define CHAT_SEND_BUTTON_INITIAL_Y                          LOBBY_NAME_INITIAL_Y - 20
#define CHAT_SEND_BUTTON_WIDTH                              160
#define CHAT_SEND_BUTTON_HEIGHT                             30
#define CHAT_SEND_BUTTON_PADDING                            25


/* States */
#define DRAW_JOIN                                           0
#define DRAW_MAIN_MENU                                      1
#define DRAW_LOBBY                                          2
#define DRAW_GAME_STATE                                     3
#define DRAW_STATISTICS                                     4

#define JOIN_BUTTON_TEXT                                    "Join"
#define ONE_V_ONE_BUTTON_TEXT                               "1V1"
#define TWO_V_TWO_BUTTON_TEXT                               "2v2"

#define INPUT_MESSAGE_MAX_LEN                               256

#define SCORE_HEIGHT                                        50
#define SCORE_1_X                                           (WINDOW_WIDTH / 4)
#define SCORE_2_X                                           (WINDOW_WIDTH - SCORE_1_X)

#define INPUTS_PER_SEC                                      1/20.0

#define LOBBY_NAME_SPACING                                  55
#define LOBBY_NAME_X                                        30
#define LOBBY_NAME_INITIAL_Y                                100

#define MAX_MESSAGE_LIST_SIZE                               5 
#define CHAT_LINE_SPACING                                   30
#define CHAT_MESSAGE_X                                      (CHAT_WINDOW_X + 20)
#define CHAT_MESSAGE_INITIAL_Y                              (CHAT_WINDOW_Y + 30)

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

/* RGB Colors */
#define RGB_WHITE                                           ((RGB) {1.0, 1.0, 1.0})
#define RGB_BLACK                                           ((RGB) {0.0, 0.0, 0.0})
#define RGB_RED                                             ((RGB) {1.0, 0.0, 0.0})
#define RGB_GREEN                                           ((RGB) {0.0, 1.0, 0.0})
#define RGB_BLUE                                            ((RGB) {0.0, 0.0, 1.0})
#define RGB_YELLOW                                          ((RGB) {1.0, 1.0, 0.5})                                          
#define RGB_PURPLE                                          ((RGB) {1.0, 0.0, 1.0})

/* Object colors */
#define CHAT_MESSAGE_COLOR                                  RGB_WHITE
#define INFO_MESSAGE_COLOR                                  RGB_BLUE 
#define ERROR_MESSAGE_COLOR                                 RGB_RED 
#define CHAT_OUTLINE_COLOR                                  RGB_YELLOW
#define TEXT_FRAME_COLOR                                    RGB_YELLOW
#define TEXT_COLOR                                          RGB_WHITE
#define NAME_COLOR                                          RGB_WHITE
#define PADDLE_COLOR                                        RGB_WHITE
#define SCORE_COLOR                                         RGB_WHITE

typedef struct _render_info {
    char state;
    char prev_state;
    char *data;
    char input_buf[INPUT_MESSAGE_MAX_LEN + 1];
    int textlen;
    char keys[3];
    char message_list_size;
    lnode *message_list_head;
    char client_ids_in_lobby[4];
} render_info;

typedef struct _RGB_ {
    float r;
    float g;
    float b;
} RGB;

// extern render_info rend_info;
// typedef struct _DRAW_INFO_ {
//     char state;
//     char buffer[256];
//     int bufsize;
//     char *data;
// } Draw_Info;


/* Helpers */
float coords_to_screen_location(float coord, char coord_type);
void print_input_buf();
void empty_input_buffer();

/* Keyboards */
void type_keyboard(unsigned char key, int x, int y);
void game_keyboard(unsigned char key, int x, int y);

/* Drawing Functions */
void draw_rectangle(int x, int y, int width, int height, RGB color);
void draw_outline(int x, int y, int width, int height, RGB rgb);
void render_string(int x, int y, char* string, RGB rgb); /* currently not working */
void render_char(int x, int y, const unsigned char c, RGB rgb);
void draw_input_buffer(int x, int y, RGB rgb);
void render_chat_message(lnode *mnode, int x, int y);
void render_int(int x, int y, int number, RGB color);
void render_int_with_tag(int x, int y, char *tag, int num, RGB color);

/* Button Handling */
void draw_button(int x, int y, int width, int height, char* text, void (*callback)(int, int, int, int));
int button_pressed(int btn_x, int btn_y, int btn_width, int btn_height, int mouse_x, int mouse_y);
void join_button_listener(int button, int event, int x, int y);
void game_type_button_listener(int button, int event, int x, int y);
void lobby_send_message_listener(int button, int event, int x, int y);

/* Views */
void draw_initial_screen();
void draw_game_type_view();
void draw_game_state();
void draw_statistics();

/* */
void render();
void gameloop();


#endif