#include "../include/game.h"

int main(void) {
    GameState state = init();

    while (state.run) {
        process_input(&state);
        update(&state);
        render(&state);
    }

    cleanup(&state);
    return 0;
}

void process_input(GameState *state) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            state->run = false;
            return;
        } else {
            switch (state->current_scene) {
            case START:
                switch (event.type) {
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        state->run = false;
                        return;
                    case SDL_SCANCODE_SPACE:
                        state->current_scene = GAME;
                        return;
                    default:
                        return;
                    }
                default:
                    return;
                }
            case GAME:
                switch (event.type) {
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        state->current_scene = PAUSE;
                        return;
                    case SDL_SCANCODE_SPACE:
                        state->current_scene = GAME;
                        return;
                    default:
                        if (state->current_scene == GAME && event.key.scancode >= SDL_SCANCODE_A && event.key.scancode <= SDL_SCANCODE_Z) {
                            char pressed_letter = 'A' + (event.key.scancode - SDL_SCANCODE_A);

                            // TODO: this needs work.
                            // need some sort of "highest current balloon" pointer to begin loop
                            // ring-type behavior needed on state->balloons array
                            for (int i = 0; i < state->max_balloons; i++) {
                                Balloon *b = &state->balloons[i];
                                if (b->active && b->letters[0] == pressed_letter) {
                                    b->pop = true;
                                    break;
                                }
                                if (i == state->max_balloons - 1) {
                                    state->health--;
                                };
                            }
                        }
                    }
                default:
                    return;
                }
            case PAUSE:
                switch (event.type) {
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        state->run = false;
                        return;
                    case SDL_SCANCODE_SPACE:
                        state->current_scene = GAME;
                        return;
                    default:
                        return;
                    }
                default:
                    return;
                }
            case GAMEOVER:
                switch (event.type) {
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        state->run = false;
                        return;
                    case SDL_SCANCODE_SPACE:
                        reset(state);
                        return;
                    default:
                        return;
                    }
                default:
                    return;
                }
            }
        }
    }
}

void update(GameState *state) {
    if (state->health == 0) {
        state->current_scene = GAMEOVER;
        return;
    }
    if (state->current_scene == GAME) {
        if (state->init_balloons) {
            init_balloons(state);
            state->init_balloons = false;
        }
        for (int i = 0; i < state->max_balloons; i++) {
            Balloon *b = &state->balloons[i];
            // TODO: after art change, verify balloon active-ness
            if (!b->active && b->y + (BALLOON_HEIGHT / 2.) < HEIGHT) {
                b->active = true;
            }

            if (b->pop) {
                b->active = false;
                b->x = rand() % (WIDTH - BALLOON_WIDTH);
                b->y = HEIGHT + (rand() % 200);
                b->letters[0] = 'A' + (rand() % 26);
                SDL_DestroyTexture(b->letter_texture);
                b->letter_texture = create_text_texture(state->renderer, state->font, b->letters);
                b->pop = false;
                state->score++;
            }

            b->y += b->y_vel;

            if (b->y + BALLOON_HEIGHT / 2. < 0) {
                b->active = false;
                b->x = rand() % (WIDTH - BALLOON_WIDTH);
                b->y = HEIGHT + (rand() % 200);
                b->letters[0] = 'A' + (rand() % 26);
                SDL_DestroyTexture(b->letter_texture);
                b->letter_texture = create_text_texture(state->renderer, state->font, b->letters);
                b->pop = false;
                state->score--;
            }
        }
    }

    if (state->current_scene != PAUSE) {
        state->scroll_offset += BG_SCROLL_VEL;
        if (state->scroll_offset >= WIDTH) {
            state->scroll_offset -= WIDTH;
        }
    }
}

void render(GameState *state) {
    // draw the background (ALL SCENES)
    draw_background(state);

    switch (state->current_scene) {
    case GAME: {
        draw_health(state);
        for (int i = 0; i < state->max_balloons; i++) {
            Balloon *b = &state->balloons[i];

            SDL_FRect rect = {b->x,
                              b->y,
                              BALLOON_WIDTH,
                              BALLOON_HEIGHT};
            SDL_RenderTexture(state->renderer, state->balloon_texture, NULL, &rect);

            rect = (SDL_FRect){b->x + BALLOON_WIDTH / 2.336,
                               b->y + BALLOON_HEIGHT / 2.223,
                               BALLOON_WIDTH / 10.,
                               BALLOON_HEIGHT / 10.};
            SDL_RenderTexture(state->renderer, b->letter_texture, NULL, &rect);
        }

        SDL_FRect rect = {0, 0, 100, 25};
        snprintf(state->score_text, 25, "Score: %d", state->score);
        SDL_Texture *texture = create_text_texture(state->renderer, state->font, state->score_text);
        SDL_RenderTexture(state->renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
        break;
    }
    case START: {
        float mul_factor;
        if (state->frame_count <= 30) {
            mul_factor = 1 + (state->frame_count + 1) * 1 / 30.;
        } else {
            mul_factor = 3 - (state->frame_count + 1) * 1 / 30.;
        }
        int start_width = mul_factor * 300;
        int start_height = mul_factor * .8 * 45;
        SDL_FRect rect = {WIDTH / 2. - start_width / 2.,
                          HEIGHT / 2. - start_height / 2.,
                          start_width,
                          start_height};
        SDL_RenderTexture(state->renderer, state->start_texture, NULL, &rect);
        break;
    }
    case PAUSE: {
        draw_health(state);

        int pause_width = 350;
        int pause_height = 45;
        SDL_FRect rect = {WIDTH / 2. - pause_width / 2.,
                          HEIGHT / 2. - pause_height / 2. - 35,
                          pause_width,
                          pause_height};
        SDL_RenderTexture(state->renderer, state->pause_textures[0], NULL, &rect);

        rect = (SDL_FRect){WIDTH / 2. - pause_width / 2.,
                           HEIGHT / 2. - pause_height / 2. + 35,
                           pause_width,
                           pause_height};
        SDL_RenderTexture(state->renderer, state->pause_textures[1], NULL, &rect);
        break;
    }
    case GAMEOVER: {
        float mul_factor;
        if (state->frame_count <= 30) {
            mul_factor = 1 + (state->frame_count + 1) * 1 / 30.;
        } else {
            mul_factor = 3 - (state->frame_count + 1) * 1 / 30.;
        }
        int menu_width = mul_factor * 200;
        int menu_height = mul_factor * .8 * 45;
        SDL_FRect rect = {WIDTH / 2. - menu_width / 2.,
                          HEIGHT / 2. - menu_height / 2.,
                          menu_width,
                          menu_height};
        SDL_RenderTexture(state->renderer, state->gameover_textures[0], NULL, &rect);

        int pause_width = 300;
        int pause_height = 30;
        rect = (SDL_FRect){WIDTH / 2. - pause_width / 2.,
                           HEIGHT / 2. - pause_height / 2. + 60,
                           pause_width,
                           pause_height};
        SDL_RenderTexture(state->renderer, state->gameover_textures[1], NULL, &rect);

        rect = (SDL_FRect){WIDTH / 2. - pause_width / 2.,
                           HEIGHT / 2. - pause_height / 2. + 95,
                           pause_width,
                           pause_height};

        SDL_RenderTexture(state->renderer, state->gameover_textures[2], NULL, &rect);
        break;
    }
    }

    SDL_RenderPresent(state->renderer);

    // Frame limiting
    uint64_t elapsed_time = SDL_GetTicksNS() - state->last_frame_time;
    if (elapsed_time < NS_PER_FRAME)
        SDL_DelayNS(NS_PER_FRAME - elapsed_time);
    state->last_frame_time = SDL_GetTicksNS();
    state->frame_count++;
    state->frame_count %= 60;
}

SDL_Texture *create_text_texture(SDL_Renderer *renderer, TTF_Font *font, char *text) {
    SDL_Color color = {0, 0, 0, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, (size_t)NULL, color);
    if (!surface) {
        fprintf(stderr, "Failed to render text: %s\n", SDL_GetError());
        fprintf(stderr, "%s\n", text);
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
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

    state.font = TTF_OpenFont("assets/font.ttf", FONT_SIZE);
    if (!state.font) {
        fprintf(stderr, "Failed to load font: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    state.window = SDL_CreateWindow("Alphabet Balloons", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    state.renderer = SDL_CreateRenderer(state.window, NULL);
    SDL_SetRenderLogicalPresentation(state.renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    state.balloon_texture = IMG_LoadTexture(state.renderer, "assets/balloon.png");
    state.background = IMG_LoadTexture(state.renderer, "assets/background.jpg");
    state.heart = IMG_LoadTexture(state.renderer, "assets/heart.png");
    state.empty_heart = IMG_LoadTexture(state.renderer, "assets/heart_empty.png");

    state.start_texture = create_text_texture(state.renderer, state.font, "Press space to start!\0");
    state.pause_textures[0] = create_text_texture(state.renderer, state.font, "Press space to resume!\0");
    state.pause_textures[1] = create_text_texture(state.renderer, state.font, "Press escape to exit!\0");
    state.gameover_textures[0] = create_text_texture(state.renderer, state.font, "Game Over!\0");
    state.gameover_textures[1] = create_text_texture(state.renderer, state.font, "Press space to try again!\0");
    state.gameover_textures[2] = create_text_texture(state.renderer, state.font, "Press escape to exit!\0");

    state.run = true;
    state.last_frame_time = SDL_GetTicksNS();
    state.score = 0;
    state.current_scene = START;
    // TODO: make health selectable on main screen;
    state.health = 5;
    state.max_health = 5;
    state.init_balloons = true;
    state.max_balloons = 10;
    state.balloon_speed = -1.7f;

    return state;
}

void cleanup(GameState *state) {
    if (!state->init_balloons) {
        for (int i = 0; i < state->max_balloons; i++) {
            SDL_DestroyTexture(state->balloons[i].letter_texture);
        }
        free(state->balloons);
    }
    TTF_CloseFont(state->font);
    SDL_DestroyTexture(state->background);
    SDL_DestroyTexture(state->heart);
    SDL_DestroyTexture(state->empty_heart);
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    TTF_Quit();
    SDL_Quit();
}

void reset(GameState *state) {
    for (int i = 0; i < state->max_balloons; i++) {
        SDL_DestroyTexture(state->balloons[i].letter_texture);
    }

    int y_gap = HEIGHT / state->max_balloons;
    for (int i = 0; i < state->max_balloons; i++) {
        Balloon *b = &state->balloons[i];
        b->x = rand() % (WIDTH - BALLOON_WIDTH);
        b->y = HEIGHT + (i * y_gap);
        b->x_vel = 0;
        b->letters[0] = 'A' + (rand() % 26);
        b->letters[1] = '\0';
        b->letter_texture = create_text_texture(state->renderer, state->font, b->letters);
        b->active = false;
        b->pop = false;
    }

    state->run = true;
    state->last_frame_time = SDL_GetTicksNS();
    state->score = 0;
    state->current_scene = GAME;
    state->health = state->max_health;
}

void draw_health(GameState *state) {
    for (int i = 1; i <= state->max_health; i++) {
        SDL_FRect rect = {WIDTH - 48 * i, 0, 48, 48};
        if (i <= state->health) {
            SDL_RenderTexture(state->renderer, state->heart, NULL, &rect);
        } else {
            SDL_RenderTexture(state->renderer, state->empty_heart, NULL, &rect);
        }
    }
}

void draw_background(GameState *state) {
    int bg1_x = (int)state->scroll_offset % WIDTH;
    int bg2_x = bg1_x - WIDTH;
    SDL_FRect srcRect = {0, 0, WIDTH, HEIGHT};
    SDL_FRect destRect1 = {bg1_x, 0, WIDTH, HEIGHT};
    SDL_FRect destRect2 = {bg2_x, 0, WIDTH, HEIGHT};
    SDL_RenderTexture(state->renderer, state->background, &srcRect, &destRect1);
    SDL_RenderTexture(state->renderer, state->background, &srcRect, &destRect2);
}

void init_balloons(GameState *state) {
    // cant init this til after the MENU scene
    state->balloons = malloc(sizeof(Balloon) * state->max_balloons);
    int y_gap = HEIGHT / state->max_balloons;
    for (int i = 0; i < state->max_balloons; i++) {
        Balloon *b = &state->balloons[i];
        b->x = rand() % (WIDTH - BALLOON_WIDTH);
        b->y = HEIGHT + (i * y_gap);
        b->x_vel = 0;
        b->y_vel = state->balloon_speed;
        b->letters[0] = 'A' + (rand() % 26);
        b->letters[1] = '\0';
        b->letter_texture = create_text_texture(state->renderer, state->font, b->letters);
        b->active = false;
        b->pop = false;
    }
}
