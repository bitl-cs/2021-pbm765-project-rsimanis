#include "graphics.h"
#include <GL/freeglut_std.h>


float coords_to_screen_location(float coord, char coord_type) // coord_type is either 'x' or 'y'
{
    float ret_val;

    if(coord_type == 'x'){ // for x coordinate
        if(coord >= 0 && coord <= WINDOW_WIDHT){ // if x coord is on screen
            ret_val = ((coord / WINDOW_WIDHT) - 0.5) * 2;
        } else {
            printf("Given coordinate %f is not on a screen, returned %c coord as 0.0!\n", coord, coord_type);
        }
    } else if(coord_type == 'y'){ //for y coordinate
        if(coord >= 0 && coord <= WINDOW_HEIGHT){ //if coord is on screen
            ret_val = ((coord / WINDOW_HEIGHT) - 0.5) * 2;
        } else {
            printf("Given coordinate %f is not on a screen, returned %c coord as 0.0!\n", coord, coord_type);
        }
    } else {
        printf("No such dimension %c!\n", coord_type);
    }

    /* for debugging */
    //printf("Converted %f to %f\n", coord, ret_val);

    return ret_val;

}

void join_button_listener(int button, int event, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(play_button_x, play_button_y, play_button_width, play_button_height, x, y)){
            printf("CLicked JOIN!\n");

            /* Send join packet to server */
        }
    }
}

void game_type_button_listener(int button, int event, int x, int y){
    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(one_v_one_x, one_v_one_y, game_type_button_width, game_type_button_height, x, y)){
            printf("CLicked 1V1!\n");
            /* Send game type packet to server (1v1) */
        } else if(button_pressed(two_v_two_x, two_v_two_x, game_type_button_width, game_type_button_height, x, y)){
            printf("Clicked 2V2!\n");
            /* Send game type packet to server (2v2) */
        }
            
        
    }
}


void draw_rectangle(int x, int y, int width, int height, RGB rgb)
{
    
    glPointSize(3);
    glColor3f(rgb.r, rgb.g, rgb.b);

    glBegin(GL_POLYGON);
        glVertex2f(coords_to_screen_location(x + width, 'x'), coords_to_screen_location(y, 'y'));
        glVertex2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y, 'y'));
        glVertex2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y + height, 'y'));
        glVertex2f(coords_to_screen_location(x + width, 'x'), coords_to_screen_location(y + height, 'y'));
    glEnd();
    glFlush();
}

void draw_outline(int x, int y, int width, int height, RGB rgb)
{
    
    glPointSize(3);
    glColor3f(rgb.r, rgb.g, rgb.b);

    glBegin(GL_LINE_LOOP);
        glVertex2f(coords_to_screen_location(x + width, 'x'), coords_to_screen_location(y, 'y'));
        glVertex2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y, 'y'));
        glVertex2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y + height, 'y'));
        glVertex2f(coords_to_screen_location(x + width, 'x'), coords_to_screen_location(y + height, 'y'));
    glEnd();
    glFlush();
}

int button_pressed(int btn_x, int btn_y, int btn_width, int btn_height, int mouse_x, int mouse_y){
    if(mouse_x <= btn_x + btn_width && mouse_x >= btn_x && mouse_y >= btn_y && mouse_y <= btn_y + btn_height){
        return 1;
    } else {
        return 0;
    }
}

void draw_initial_screen(){ // no pointer to data here needed
    
    /* Play Button Data */
    RGB play_button_color = {1.0f, 1.0f, 0.0f};

    int space_between_components = 5;
    /* Text Frame Data */
    int text_frame_width = 160;
    int text_frame_height = 40;
    int text_frame_x = (WINDOW_WIDHT - text_frame_width) / 2;
    int text_frame_y = ((WINDOW_HEIGHT - text_frame_height) / 2) + space_between_components + text_frame_height;
    RGB text_frame_color = {1.0f, 0.0f, 0.0f};
    /* Button listener and place */
    glutMouseFunc(join_button_listener);
    draw_rectangle(play_button_x, play_button_y, play_button_width, play_button_height, play_button_color);
    /* Text Frame */
    draw_outline(text_frame_x, text_frame_y, text_frame_width, text_frame_height, text_frame_color);
}


void render_string(float x, float y, void *font, const char* string, RGB rgb) //currently development in progress
{  
//   char *c;

//   glColor3f(rgb.r, rgb.g, rgb.b); 
//   glRasterPos2f(x, y);

//   glutBitmapString(font, string);
}

void draw_game_type_view(){
    /* Button Data */
    RGB color = {1.0f, 1.0f, 1.0f};
    
    
    /* Drawing Buttons and adding listener */
    glutMouseFunc(game_type_button_listener);
    draw_rectangle(one_v_one_x, one_v_one_y, game_type_button_width, game_type_button_height, color);
    draw_rectangle(two_v_two_x, two_v_two_y, game_type_button_width, game_type_button_height, color);
}

// void init_screen(int argc, char **argv) {
//     glutInit(&argc, argv);
//     glutInitWindowPosition(250, 200);
//     glutInitWindowSize(WINDOW_WIDHT, WINDOW_HEIGHT);
//     glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
//     glutCreateWindow("Test OpenGL program");

//     draw_initial_screen();
//     glutMainLoop();
// }

// int main(int argc, char** argv){
    
//     glutInit(&argc, argv);
//     glutInitWindowPosition(250, 200);
//     glutInitWindowSize(WINDOW_WIDHT, WINDOW_HEIGHT);
//     glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
//     glutCreateWindow("Test OpenGL program");
//     glutDisplayFunc(draw_initial_screen);
//     glutMainLoop();

//     return 0;
// }

// void render_characters(){
//     FT_Library ft;
//     if (FT_Init_FreeType(&ft))
//         {
//             printf("ERROR::FREETYPE: Could not init FreeType Library\n");
//         }

//     FT_Face face;
//     if (FT_New_Face(ft, "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf", 0, &face))
//         {
//             printf("ERROR::FREETYPE: Failed to load font...sad!\n");
//         }
    
//     FT_Set_Pixel_Sizes(face, 0, 48);

//     if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
//     {
//         printf("ERROR::FREETYTPE: Failed to load Glyph\n");
//     }

//     glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//     glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    
//     Character characters[128];
//     for (unsigned char c = 0; c < 128; c++)
//     {
//         // load character glyph 
//         if (FT_Load_Char(face, c, FT_LOAD_RENDER))
//         {
//             printf("ERROR::FREETYTPE: Failed to load Glyph\n");
//             continue;
//         }
//         // generate texture
//         unsigned int texture;
//         glGenTextures(1, &texture);
//         glBindTexture(GL_TEXTURE_2D, texture);
//         glTexImage2D(
//             GL_TEXTURE_2D,
//             0,
//             GL_RED,
//             face->glyph->bitmap.width,
//             face->glyph->bitmap.rows,
//             0,
//             GL_RED,
//             GL_UNSIGNED_BYTE,
//             face->glyph->bitmap.buffer
//         );
//         // set texture options
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//         // now store character for later use
        
        
//         int size_vec[2];
//         size_vec[0] = face->glyph->bitmap.width; size_vec[1] = face->glyph->bitmap.rows;
        
//         int bearing_vec[2] = {face->glyph->bitmap_left, face->glyph->bitmap_top};
        

//         Character character = {texture, size_vec, bearing_vec, face->glyph->advance.x};

//         characters[c] = character;
//     }

//     glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
//     FT_Done_Face(face);
//     FT_Done_FreeType(ft); 

//     #version 330 core
//     layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
//     out vec2 TexCoords;

//     union mat4 projection;

// }
