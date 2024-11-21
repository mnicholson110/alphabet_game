#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TARGET_FPS 60
#define NS_PER_FRAME (1. / TARGET_FPS) * 1000000000
#define WIDTH 800
#define HEIGHT 600
#define BG_SCROLL_VEL 1.2f
#define BALLOON_WIDTH 250
#define BALLOON_HEIGHT 250
#define FONT_SIZE 96

typedef enum {
    START,
    GAME,
    GAMEOVER,
    MENU,
    PAUSE
} Scene;

typedef struct {
    float x;
    float y;
    SDL_Texture *letter_texture;
    char letters[2];
    bool active;
    bool pop;
} Balloon;

typedef struct {
    bool run;
    uint64_t last_frame_time;
    uint8_t frame_count;
    float scroll_offset;
    uint32_t score;
    char score_text[11];
    Scene current_scene;
    uint8_t health;
    uint8_t max_health;
    uint8_t max_balloons;
    bool init_balloons;
    float balloon_speed;
    Balloon *balloons;
    SDL_Texture *background;
    SDL_Texture *balloon_texture;
    SDL_Texture *start_texture;
    SDL_Texture *pause_textures[2];
    SDL_Texture *gameover_textures[3];
    SDL_Texture *menu_header_textures[3];
    SDL_Texture *menu_selection_textures[9];
    SDL_Texture *heart;
    SDL_Texture *empty_heart;
    SDL_Texture *arrow;
    TTF_Font *font;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_FRect menu_selection_rects[3];
    uint8_t current_selection;
} GameState;

GameState init(void);
void reset(GameState *state);
void cleanup(GameState *state);
void process_input(GameState *state);
void update(GameState *state);
void render(GameState *state);
SDL_Texture *create_text_texture(SDL_Renderer *renderer, TTF_Font *font, char *text);
void draw_background(GameState *state);
void draw_health(GameState *state);
void init_balloons(GameState *state);
void set_variables(GameState *state);
