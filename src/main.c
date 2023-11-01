#include <SDL.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *surface;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PIXEL_WIDTH 20
#define PIXEL_HEIGHT 20
#define GRID_WIDTH (WINDOW_WIDTH / PIXEL_WIDTH)
#define GRID_HEIGHT (WINDOW_HEIGHT / PIXEL_HEIGHT)

#define BRUSH_COLOR 0x33CCBBFF
#define BG_COLOR 0xCCCCCCFF

typedef struct {
    bool running;
    bool left_mouse_down;
    bool right_mouse_down;
    bool grid_changed;
} PAG_Settings;

PAG_Settings PAG_Settings_default() {
    PAG_Settings s;
    s.running = true;
    s.left_mouse_down = false;
    s.right_mouse_down = false;
    s.grid_changed = true;
    return s;
}

void check(int result, char *msg) {
    if (result < 0) {
        printf("Error during %s: %s\n", msg, SDL_GetError());
        exit(1);
    }
}

void *check_ptr(void *result, char *msg) {
    if (result == NULL) {
        printf("Error during %s: %s\n", msg, SDL_GetError());
        exit(1);
    }

    return result;
}

void cleanup() {
    SDL_DestroySurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void colorPixel(uint32_t *grid, size_t x, size_t y, uint32_t color) {
    if (0 <= x && x < GRID_WIDTH && 0 <= y && y < GRID_HEIGHT) {
        grid[y * GRID_WIDTH + x] = color;
    }
}

void handleSDLEvents(PAG_Settings *settings, uint32_t *grid) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_EVENT_QUIT: {
                settings->running = false;
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                uint32_t color;
                if (e.button.button == SDL_BUTTON_LEFT) {
                    color = BRUSH_COLOR;
                    settings->left_mouse_down = true;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    color = BG_COLOR;
                    settings->right_mouse_down = true;
                } else {
                    continue;
                }

                colorPixel(
                    grid,
                    e.button.x / PIXEL_WIDTH,
                    e.button.y / PIXEL_HEIGHT,
                    color
                );
                settings->grid_changed = true;

                continue;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    settings->left_mouse_down = false;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    settings->right_mouse_down = false;
                }
                continue;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                if (!settings->left_mouse_down && !settings->right_mouse_down)
                    continue;

                colorPixel(
                    grid,
                    e.button.x / PIXEL_WIDTH,
                    e.button.y / PIXEL_HEIGHT,
                    settings->left_mouse_down ? BRUSH_COLOR : BG_COLOR
                );
                settings->grid_changed = true;

                continue;
            }
        }
    }
}

void renderPixelGrid(const uint32_t *grid) {
    for (size_t y = 0; y < GRID_HEIGHT; y++) {
        for (size_t x = 0; x < GRID_WIDTH; x++) {
            SDL_FillSurfaceRect(
                surface,
                &(SDL_Rect){
                    .x = x * PIXEL_WIDTH,
                    .y = y * PIXEL_HEIGHT,
                    .w = PIXEL_WIDTH,
                    .h = PIXEL_HEIGHT
                },
                grid[y * GRID_WIDTH + x]
            );
        }
    }
    SDL_Texture *texture = check_ptr(SDL_CreateTextureFromSurface(renderer, surface), "Texture creation");
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);
}

int main(int argc, char *argv[])
{
    assert(WINDOW_WIDTH % PIXEL_WIDTH == 0);
    assert(WINDOW_HEIGHT % PIXEL_HEIGHT == 0);

    uint32_t *pixel_grid = malloc(GRID_WIDTH * GRID_HEIGHT * sizeof(uint32_t));
    for (size_t y = 0; y < GRID_HEIGHT; y++) {
        for (size_t x = 0; x < GRID_WIDTH; x++) {
            pixel_grid[y * GRID_WIDTH + x] = BG_COLOR;
        }
    }

    check(SDL_Init(SDL_INIT_VIDEO), "SDL initialization");
    atexit(cleanup);
    window = check_ptr(SDL_CreateWindow("PixelArtGenerator", WINDOW_WIDTH, WINDOW_HEIGHT, 0), "Window creation");
    renderer = check_ptr(SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED), "Renderer creation");
    surface = check_ptr(SDL_CreateSurface(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_PIXELFORMAT_RGBA8888), "Surface creation");

    PAG_Settings settings = PAG_Settings_default();
    while (settings.running) {
        if (settings.grid_changed) {
            renderPixelGrid(pixel_grid);
            SDL_RenderPresent(renderer);
            settings.grid_changed = false;
        }

        handleSDLEvents(&settings, pixel_grid);
    }

	return 0;
}