#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include<GL/glu.h>
#include<GL/glut.h>
#include<stdio.h>
//#include <ft2build.h>   /* For Text Rendering */ | currently not needed
//#include FT_FREETYPE_H  /* For Text Rendering */ | currently not needed
//#include <openglut.h>

#define WINDOW_HEIGHT                                       600
#define WINDOW_WIDHT                                        800 

#define play_button_width                                   80
#define play_button_height                                  30
#define play_button_x                                       (WINDOW_WIDHT - play_button_width) / 2
#define play_button_y                                       (WINDOW_HEIGHT - play_button_height) / 2

#define game_type_button_width                              160
#define game_type_button_height                             60
#define horizontal_offset_between_game_type_buttons         25
#define vertical_offset_between_game_type_buttons           80

/* 1V1 */
#define one_v_one_x  (WINDOW_WIDHT / 2 - game_type_button_width - horizontal_offset_between_game_type_buttons)
#define one_v_one_y  (WINDOW_HEIGHT / 2 - vertical_offset_between_game_type_buttons)

/* 2V2 */
#define two_v_two_x  (WINDOW_WIDHT / 2 + horizontal_offset_between_game_type_buttons)
#define two_v_two_y  (WINDOW_HEIGHT / 2 - vertical_offset_between_game_type_buttons)


typedef struct RGB{
    float r;
    float g;
    float b;
} RGB;

typedef struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    int size[2];       // Size of glyph as vector 2d
    int Bearing[2];    // Offset from baseline to left/top of glyph as vector 2d
    unsigned int Advance;    // Offset to advance to next glyph
} Character;

/* Helpers */
float coords_to_screen_location(float coord, char coord_type);

/* Drawing Functions */
void draw_rectangle(int x, int y, int width, int height, RGB color);
void draw_outline(int x, int y, int width, int height, RGB rgb);
void render_string(float x, float y, void *font, const char* string, RGB rgb); /* currently not working */

/* Button Handling */
int button_pressed(int btn_x, int btn_y, int btn_width, int btn_height, int mouse_x, int mouse_y);
void join_button_listener(int button, int event, int x, int y);
void game_type_button_listener(int button, int event, int x, int y);

/* Views */
void draw_initial_screen();
void draw_game_type_view();
void draw_game_state(char *data);
void draw_statistics();

#endif