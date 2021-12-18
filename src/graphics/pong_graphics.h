#ifndef _PONG_GRAPHICS_H
#define _PONG_GRAPHICS_H

#include "../utils/message_list.h"
#include "../networking/pong_networking.h"

#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <GL/freeglut_std.h>

/* Window */
#define WINDOW_HEIGHT                                       800
#define WINDOW_WIDTH                                        800 

/* RGB Colors */
#define RGB_WHITE                                           ((RGB) {1.0, 1.0, 1.0})
#define RGB_BLACK                                           ((RGB) {0.0, 0.0, 0.0})
#define RGB_RED                                             ((RGB) {1.0, 0.1, 0.1})
#define RGB_GREEN                                           ((RGB) {0.0, 1.0, 0.0})
#define RGB_BLUE                                            ((RGB) {0.1, 0.1, 1.0})
#define RGB_YELLOW                                          ((RGB) {1.0, 1.0, 0.5})                                          
#define RGB_PURPLE                                          ((RGB) {1.0, 0.0, 1.0})

/* Chat */
#define CHAT_WINDOW_X                                       CHAT_WINDOW_PADDING                      
#define CHAT_WINDOW_Y                                       600
#define CHAT_WINDOW_WIDTH                                   (WINDOW_WIDTH - (2 * CHAT_WINDOW_PADDING))
#define CHAT_WINDOW_HEIGHT_WITH_INPUT_FIELD                 (CHAT_WINDOW_HEIGHT_WITHOUT_INPUT_FIELD - CHAT_INPUT_FIELD_HEIGHT - 2 * CHAT_INPUT_FIELD_PADDING)
#define CHAT_WINDOW_HEIGHT_WITHOUT_INPUT_FIELD              (200 - CHAT_WINDOW_PADDING)
#define CHAT_WINDOW_PADDING                                 10
#define CHAT_OUTLINE_COLOR                                  RGB_YELLOW
#define CHAT_MESSAGE_LINE_SPACING                           30
#define CHAT_MESSAGE_X                                      (CHAT_WINDOW_X + 20)
#define CHAT_MESSAGE_INITIAL_Y                              (CHAT_WINDOW_Y + 27)
#define CHAT_DISPLAYED_MESSAGE_COUNT_WITH_INPUT_FIELD       4
#define CHAT_DISPLAYED_MESSAGE_COUNT_WITHOUT_INPUT_FIELD    6

#define CHAT_NORMAL_MESSAGE_COLOR                           RGB_WHITE
#define CHAT_INFO_MESSAGE_COLOR                             RGB_BLUE 
#define CHAT_ERROR_MESSAGE_COLOR                            RGB_RED 

#define CHAT_INPUT_FIELD_X                                  CHAT_INPUT_FIELD_PADDING
#define CHAT_INPUT_FIELD_Y                                  (WINDOW_HEIGHT - CHAT_INPUT_FIELD_HEIGHT - CHAT_INPUT_FIELD_PADDING)
#define CHAT_INPUT_FIELD_WIDTH                              (WINDOW_WIDTH - CHAT_SEND_ALL_BUTTON_WIDTH - 3 * CHAT_SEND_ALL_BUTTON_PADDING)
#define CHAT_INPUT_FIELD_HEIGHT                             30
#define CHAT_INPUT_FIELD_PADDING                            10

#define CHAT_SEND_ALL_BUTTON_X                              (WINDOW_WIDTH - CHAT_SEND_ALL_BUTTON_WIDTH - CHAT_SEND_ALL_BUTTON_PADDING)
#define CHAT_SEND_ALL_BUTTON_Y                              CHAT_INPUT_FIELD_Y
#define CHAT_SEND_ALL_BUTTON_WIDTH                          140
#define CHAT_SEND_ALL_BUTTON_HEIGHT                         CHAT_INPUT_FIELD_HEIGHT
#define CHAT_SEND_ALL_BUTTON_PADDING                        10

#define CHAT_SEND_BUTTON_X                                  (LOBBY_NAME_FIELD_X + 200)
#define CHAT_SEND_BUTTON_INITIAL_Y                          LOBBY_NAME_FIELD_INITIAL_Y - 20
#define CHAT_SEND_BUTTON_WIDTH                              160
#define CHAT_SEND_BUTTON_HEIGHT                             30
#define CHAT_SEND_BUTTON_PADDING                            25


/* Quit Button */
#define QUIT_BUTTON_X                                       (WINDOW_WIDTH - QUIT_BUTTON_WIDTH - QUIT_BUTTON_PADDING)
#define QUIT_BUTTON_Y                                       QUIT_BUTTON_PADDING
#define QUIT_BUTTON_WIDTH                                   80
#define QUIT_BUTTON_HEIGHT                                  30
#define QUIT_BUTTON_PADDING                                 10

/* Input Buffer */
#define INPUT_TEXT_MAX_LENGTH                               256

typedef struct _render_info {
    char client_id;
    char state;
    char *data;
    void *send_mem;
    char input_buf[INPUT_TEXT_MAX_LENGTH + 1];
    int input_text_len;
    char keys[3];
    char max_displayed_message_count;
    char message_list_size;
    mnode *message_list_head;
    char client_ids_in_lobby[MAX_PLAYER_COUNT];
    char client_names_in_lobby[MAX_PLAYER_COUNT][MAX_NAME_LENGTH + 1];
    clock_t last_update;
    int frame_counter;
} render_info;

typedef struct _RGB {
    float r;
    float g;
    float b;
} RGB;

extern render_info rend_info;


/* Keyboards */
void type_keyboard(unsigned char key, int x, int y);
void game_keyboard(unsigned char key, int x, int y);

/* Shapes */
void render_outline(int x, int y, int width, int height, RGB color);
void render_rectangle(int x, int y, int width, int height, RGB color);
void render_circle(int x, int y, int radius, RGB color);

/* Strings */
void render_char(int x, int y, const unsigned char c, RGB color);
void render_string(int x, int y, char* string, RGB color);
void render_string_in_the_middle(int y, char *string, RGB color);
void render_int(int x, int y, int number, RGB color);
void render_int_with_tag(int x, int y, char *tag, int num, RGB color);
int bitmapped_string_length(char *text);
int bitmapped_string_height();

/* Chat */
void render_chat_message_window();
void render_chat_message(mnode *mnode, int x, int y);
void render_chat_input_field();
void render_input_buffer(int x, int y, int maxlen, RGB color);
void clear_input_buffer();
void pop_front_message_from_chat();
void append_message_to_chat(char type, char *message);
void append_chat_message_to_chat(char source_id, char *message);
void append_info_message_to_chat(char *message);
void append_error_message_to_chat(char *message);
int match_id_to_index(char source_id);
void clear_chat();

/* Button Handling */
int button_pressed(int btn_x, int btn_y, int btn_width, int btn_height, int mouse_x, int mouse_y);
void render_button(int x, int y, int width, int height, char* text);
void render_quit_button();

/* Helpers */
float ctosl(float coord, char coord_type);

/* Debug */
void print_input_buffer();


#endif
