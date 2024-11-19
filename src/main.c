#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TARGET_FPS 60
#define NS_PER_FRAME (1. / TARGET_FPS) * 1000000000
#define MAX_BALLOONS 10
#define WIDTH 800
#define HEIGHT 600
#define BALLOON_VELOCITY -2.f
#define BG_SCROLL_VEL 1.2f
#define BALLOON_WIDTH 250
#define BALLOON_HEIGHT 250
#define FONT_SIZE 96

typedef enum {
    MENU,
    GAME,
    PAUSE
} Scene;

typedef struct {
    float x, y, x_vel, y_vel;
    SDL_Texture *texture;
    SDL_Texture *letter_texture;
    char letter[2];
    bool active;
    bool pop;
} Balloon;

typedef struct {
    bool run;
    uint64_t last_frame_time;
    uint8_t frame_count;
    Balloon balloons[MAX_BALLOONS];
    SDL_Texture *background;
    TTF_Font *font;
    SDL_Renderer *renderer;
    SDL_Window *window;
    float scroll_offset;
    uint32_t score;
    char score_text[25];
    Scene current_scene;
} GameState;

GameState init(void);
void cleanup(GameState *state);
void process_input(GameState *state);
void update(GameState *state);
void render(GameState *state);
SDL_Texture *create_text_texture(SDL_Renderer *renderer, TTF_Font *font, char *text);

int main(int argc, char **argv) {
    GameState state = init();

    while (state.run) {
        process_input(&state);
        update(&state);
        render(&state);
    }

    cleanup(&state);
    return 0;
}

GameState init() {
    srand((unsigned int)time(NULL));
    GameState state = {0};

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if (!TTF_Init()) {
        fprintf(stderr, "Failed to initialize SDL_ttf: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    state.window = SDL_CreateWindow("Alphabet Balloons", WIDTH, HEIGHT, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);
    state.renderer = SDL_CreateRenderer(state.window, NULL);
    SDL_SetRenderLogicalPresentation(state.renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_Texture *balloon_texture = IMG_LoadTexture(state.renderer, "assets/balloon.png");

    state.background = IMG_LoadTexture(state.renderer, "assets/background.jpg");

    state.font = TTF_OpenFont("assets/font.ttf", FONT_SIZE);
    if (!state.font) {
        fprintf(stderr, "Failed to load font: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int y_gap = HEIGHT / MAX_BALLOONS;
    for (int i = 0; i < MAX_BALLOONS; i++) {
        Balloon *b = &state.balloons[i];
        b->x = rand() % (WIDTH - BALLOON_WIDTH);
        b->y = HEIGHT + (i * y_gap);
        b->x_vel = 0;
        b->y_vel = BALLOON_VELOCITY;
        b->texture = balloon_texture;
        b->letter[0] = 'A' + (rand() % 26);
        b->letter[1] = '\0';
        b->letter_texture = create_text_texture(state.renderer, state.font, b->letter);
        b->active = false;
        b->pop = false;
    }

    state.run = true;
    state.last_frame_time = SDL_GetTicksNS();
    state.score = 0;
    state.current_scene = MENU;
    SDL_HideCursor();
    return state;
}

void cleanup(GameState *state) {
    for (int i = 0; i < MAX_BALLOONS; i++) {
        SDL_DestroyTexture(state->balloons[i].letter_texture);
    }
    TTF_CloseFont(state->font);
    SDL_DestroyTexture(state->background);
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    TTF_Quit();
    SDL_Quit();
}

void process_input(GameState *state) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            state->run = false;
            return;
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                state->run = false;
                return;
            }

            SDL_Scancode scancode = event.key.scancode;
            if (state->current_scene == GAME && scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) {
                char pressed_letter = 'A' + (scancode - SDL_SCANCODE_A);

                // TODO: this needs work.
                // need some sort of "highest current balloon" pointer to begin loop
                // ring-type behavior needed on state->balloons array
                for (int i = 0; i < MAX_BALLOONS; i++) {
                    Balloon *b = &state->balloons[i];
                    if (b->active && b->letter[0] == pressed_letter) {
                        b->pop = true;
                        break;
                    }
                    if (i == MAX_BALLOONS - 1) {
                        if (state->score > 0) state->score--;
                    };
                }
            }
            if (state->current_scene == MENU && scancode == SDL_SCANCODE_SPACE) {
                state->current_scene = GAME;
            }
        }
    }
}

void update(GameState *state) {
    if (state->current_scene == GAME) {
        for (int i = 0; i < MAX_BALLOONS; i++) {
            Balloon *b = &state->balloons[i];
            // TODO: after art change, verify balloon active-ness
            if (!b->active && b->y + (BALLOON_HEIGHT / 2.) < HEIGHT) {
                b->active = true;
            }

            if (b->pop) {
                b->active = false;
                b->x = rand() % (WIDTH - BALLOON_WIDTH);
                b->y = HEIGHT + (rand() % 200);
                b->letter[0] = 'A' + (rand() % 26);
                SDL_DestroyTexture(b->letter_texture);
                b->letter_texture = create_text_texture(state->renderer, state->font, b->letter);
                b->pop = false;
                state->score++;
            }

            b->y += b->y_vel;

            if (b->y + BALLOON_HEIGHT / 2. < 0) {
                b->active = false;
                b->x = rand() % (WIDTH - BALLOON_WIDTH);
                b->y = HEIGHT + (rand() % 200);
                b->letter[0] = 'A' + (rand() % 26);
                SDL_DestroyTexture(b->letter_texture);
                b->letter_texture = create_text_texture(state->renderer, state->font, b->letter);
                b->pop = false;
                if (state->score > 0) state->score--;
            }
        }
    }

    state->scroll_offset += BG_SCROLL_VEL;
    if (state->scroll_offset >= WIDTH) {
        state->scroll_offset -= WIDTH;
    }
}

void render(GameState *state) {
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->renderer);

    int bg1_x = (int)state->scroll_offset % WIDTH;
    int bg2_x = bg1_x - WIDTH;
    SDL_FRect srcRect = {0, 0, WIDTH, HEIGHT};
    SDL_FRect destRect1 = {bg1_x, 0, WIDTH, HEIGHT};
    SDL_FRect destRect2 = {bg2_x, 0, WIDTH, HEIGHT};
    SDL_RenderTexture(state->renderer, state->background, &srcRect, &destRect1);
    SDL_RenderTexture(state->renderer, state->background, &srcRect, &destRect2);

    if (state->current_scene == GAME) {
        for (int i = 0; i < MAX_BALLOONS; i++) {
            Balloon *b = &state->balloons[i];

            SDL_FRect balloon_rect = {b->x,
                                      b->y,
                                      BALLOON_WIDTH,
                                      BALLOON_HEIGHT};
            SDL_RenderTexture(state->renderer, b->texture, NULL, &balloon_rect);

            SDL_FRect letter_rect = {b->x + BALLOON_WIDTH / 2.336,
                                     b->y + BALLOON_HEIGHT / 2.232,
                                     BALLOON_WIDTH / 10.,
                                     BALLOON_HEIGHT / 10.};
            SDL_RenderTexture(state->renderer, b->letter_texture, NULL, &letter_rect);
        }

        SDL_FRect score_rect = {0, 0, 100, 25};
        snprintf(state->score_text, 25, "Score: %d", state->score);
        SDL_Texture *texture = create_text_texture(state->renderer, state->font, state->score_text);
        SDL_RenderTexture(state->renderer, texture, NULL, &score_rect);

    } else if (state->current_scene == MENU) {
        float mul_factor;
        if (state->frame_count <= 30) {
            mul_factor = 1 + (state->frame_count + 1) * 1 / 30.;
        } else {
            mul_factor = 3 - (state->frame_count + 1) * 1 / 30.;
        }
        int menu_width = mul_factor * 200;
        int menu_height = mul_factor * .8 * 30;
        SDL_FRect menu_rect = {WIDTH / 2. - menu_width / 2.,
                               HEIGHT / 2. - menu_height / 2.,
                               menu_width,
                               menu_height};
        SDL_Texture *texture = create_text_texture(state->renderer, state->font, "Press space to start!\0");
        SDL_RenderTexture(state->renderer, texture, NULL, &menu_rect);
    }

    SDL_RenderPresent(state->renderer);

    uint64_t elapsed_time = SDL_GetTicksNS() - state->last_frame_time;
    if (elapsed_time < NS_PER_FRAME) SDL_DelayNS(NS_PER_FRAME - elapsed_time);
    state->last_frame_time = SDL_GetTicksNS();
    state->frame_count++;
    state->frame_count %= 60;
}

SDL_Texture *create_text_texture(SDL_Renderer *renderer, TTF_Font *font, char *text) {
    SDL_Color color = {0, 0, 0, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, (size_t)NULL, color);
    if (!surface) {
        fprintf(stderr, "Failed to render text: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
}
