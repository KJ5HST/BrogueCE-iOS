// BrogueCE iOS - Main Entry Point
// Adapted from Android port - removes JNI, uses iOS-compatible paths

#include "SDL.h"
#include "SDL_ttf.h"
#include "config.h"
#include "display.h"
#include "input.h"
#include "platform.h"
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "IncludeGlobals.h"

#define MAX_ERROR_LENGTH 200
#define FRAME_INTERVAL 50

struct brogueConsole currentConsole;

static SDL_Window *window;
static SDL_Rect screen;
static _Atomic boolean resumed = false;

boolean hasGraphics = true;
enum graphicsModes graphicsMode = TEXT_GRAPHICS;
boolean nonInteractivePlayback = false;
double display_scale = 1.0;  // High DPI scale factor

// Parse a uint64 from string - needed by MainMenu
boolean tryParseUint64(char *str, uint64_t *num) {
    char *endPtr;
    unsigned long long n = strtoull(str, &endPtr, 10);
    if (*endPtr != '\0') {
        return false;
    }
    *num = n;
    return true;
}

void destroy_assets() {
    SDL_SetRenderTarget(renderer, NULL);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void general_error(boolean critical, const char *error_title, const char *error_message, ...) {
    char buffer[MAX_ERROR_LENGTH];
    va_list a;
    va_start(a, error_message);
    vsnprintf(buffer, MAX_ERROR_LENGTH, error_message, a);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, error_title, buffer, NULL);
    va_end(a);
    if (critical) {
        destroy_assets();
        destroy_font();
        TTF_Quit();
        SDL_Quit();
        exit(-1);
    }
}

#define invalid_config_error(error_title, ...) general_error(true, error_title, __VA_ARGS__)

void create_assets() {
    if (SDL_CreateWindowAndRenderer(display.w, display.h,
                                    SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI,
                                    &window, &renderer)) {
        invalid_config_error("SDL Error", "Couldn't create window and renderer: %s", SDL_GetError());
    }

    // Get actual renderer output size for high DPI displays
    int render_w, render_h;
    SDL_GetRendererOutputSize(renderer, &render_w, &render_h);
    display_scale = (double)render_w / display.w;

    // Update display to use pixel dimensions for rendering
    display.w = render_w;
    display.h = render_h;

    // Recalculate cell dimensions with pixel sizes
    cell_w = ((double)display.w) / COLS;
    cell_h = ((double)display.h) / ROWS;

    // Update panel boxes with new cell dimensions
    left_panel_box = (SDL_Rect){.x = 0, .y = 0, .w = LEFT_PANEL_WIDTH * cell_w, .h = ROWS * cell_h};
    log_panel_box = (SDL_Rect){.x = LEFT_PANEL_WIDTH * cell_w, .y = 0,
                               .w = (COLS - LEFT_PANEL_WIDTH) * cell_w, .h = TOP_LOG_HEIGIHT * cell_h};
    button_panel_box = (SDL_Rect){.x = LEFT_PANEL_WIDTH * cell_w,
                                  .y = (ROWS - BOTTOM_BUTTONS_HEIGHT) * cell_h,
                                  .w = (COLS - LEFT_PANEL_WIDTH) * cell_w,
                                  .h = BOTTOM_BUTTONS_HEIGHT * cell_h};
    grid_box = grid_box_zoomed = (SDL_Rect){.x = LEFT_PANEL_WIDTH * cell_w,
                                            .y = TOP_LOG_HEIGIHT * cell_h,
                                            .w = (COLS - LEFT_PANEL_WIDTH) * cell_w,
                                            .h = (ROWS - TOP_LOG_HEIGIHT - BOTTOM_BUTTONS_HEIGHT) * cell_h};

    screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_TARGET, display.w, display.h);
    SDL_SetRenderTarget(renderer, screen_texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, COLOR_MAX);
    SDL_RenderClear(renderer);

    if (dpad_enabled) {
        // Create D-pad textures programmatically (no BMP needed)
        SDL_Surface *dpad_surface = SDL_CreateRGBSurface(0, 128, 128, 32, 0, 0, 0, 0);
        SDL_FillRect(dpad_surface, NULL, SDL_MapRGB(dpad_surface->format, 128, 128, 128));

        // Draw D-pad pattern
        SDL_Rect center = {42, 42, 44, 44};
        SDL_FillRect(dpad_surface, &center, SDL_MapRGB(dpad_surface->format, 200, 200, 200));
        SDL_Rect up = {42, 0, 44, 42};
        SDL_FillRect(dpad_surface, &up, SDL_MapRGB(dpad_surface->format, 180, 180, 180));
        SDL_Rect down = {42, 86, 44, 42};
        SDL_FillRect(dpad_surface, &down, SDL_MapRGB(dpad_surface->format, 180, 180, 180));
        SDL_Rect left = {0, 42, 42, 44};
        SDL_FillRect(dpad_surface, &left, SDL_MapRGB(dpad_surface->format, 180, 180, 180));
        SDL_Rect right = {86, 42, 42, 44};
        SDL_FillRect(dpad_surface, &right, SDL_MapRGB(dpad_surface->format, 180, 180, 180));

        dpad_image_select = SDL_CreateTextureFromSurface(renderer, dpad_surface);
        SDL_SetTextureAlphaMod(dpad_image_select, dpad_transparency);
        dpad_image_move = SDL_CreateTextureFromSurface(renderer, dpad_surface);
        SDL_SetTextureColorMod(dpad_image_move, COLOR_MAX, COLOR_MAX, 155);
        SDL_SetTextureAlphaMod(dpad_image_move, dpad_transparency);
        SDL_FreeSurface(dpad_surface);

        double area_width = min(cell_w * (LEFT_PANEL_WIDTH - 4), cell_h * 20);
        dpad_area.h = dpad_area.w = (dpad_width) ? dpad_width : area_width;
        dpad_area.x = (dpad_x_pos) ? dpad_x_pos : 3 * cell_w;
        dpad_area.y = (dpad_y_pos) ? dpad_y_pos : (display.h - (area_width + 2 * cell_h));
    }

    if (keyboard_visibility == 2) {
        start_text_input();
    }
    init_glyphs();
}

uint8_t convert_color(short c) {
    c = c * COLOR_MAX / 100;
    return max(0, min(c, COLOR_MAX));
}

int suspend_resume_filter(void *userdata, SDL_Event *event) {
    switch (event->type) {
    case SDL_APP_WILLENTERBACKGROUND:
        return 0;
    case SDL_APP_WILLENTERFOREGROUND:
        resumed = true;
        return 0;
    }
    return 1;
}

void TouchScreenGameLoop() {
    restart_game = true;
    settings_changed = false;
    do {
        if (restart_game) {
            display = screen;
            if (force_portrait) {
                SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, "Portrait PortraitUpsideDown", SDL_HINT_OVERRIDE);
                // Swap if needed to get portrait
                if (display.w > display.h) {
                    int tmp = display.w;
                    display.w = display.h;
                    display.h = tmp;
                }
            } else {
                SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight", SDL_HINT_OVERRIDE);
                // Swap if needed to get landscape (iOS reports portrait by default)
                if (display.w < display.h) {
                    int tmp = display.w;
                    display.w = display.h;
                    display.h = tmp;
                }
            }
            char render_hint[2] = {filter_mode + '0', 0};
            SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, render_hint, SDL_HINT_OVERRIDE);

            if (custom_cell_width != 0) {
                cell_w = custom_cell_width;
            } else {
                cell_w = ((double)display.w) / COLS;
            }
            if (custom_cell_height != 0) {
                cell_h = custom_cell_height;
            } else {
                cell_h = ((double)display.h) / ROWS;
            }

            left_panel_box = (SDL_Rect){.x = 0, .y = 0, .w = LEFT_PANEL_WIDTH * cell_w, .h = ROWS * cell_h};
            log_panel_box = (SDL_Rect){.x = LEFT_PANEL_WIDTH * cell_w, .y = 0,
                                       .w = (COLS - LEFT_PANEL_WIDTH) * cell_w, .h = TOP_LOG_HEIGIHT * cell_h};
            button_panel_box = (SDL_Rect){.x = LEFT_PANEL_WIDTH * cell_w,
                                          .y = (ROWS - BOTTOM_BUTTONS_HEIGHT) * cell_h,
                                          .w = (COLS - LEFT_PANEL_WIDTH) * cell_w,
                                          .h = BOTTOM_BUTTONS_HEIGHT * cell_h};
            grid_box = grid_box_zoomed = (SDL_Rect){.x = LEFT_PANEL_WIDTH * cell_w,
                                                    .y = TOP_LOG_HEIGIHT * cell_h,
                                                    .w = (COLS - LEFT_PANEL_WIDTH) * cell_w,
                                                    .h = (ROWS - TOP_LOG_HEIGIHT - BOTTOM_BUTTONS_HEIGHT) * cell_h};
            create_assets();
            if (!init_font()) {
                invalid_config_error("Font Error",
                                     "Resolution/cell size is too small for minimum allowed font size");
            }
        } else if (settings_changed) {
            create_assets();
        }
        settings_changed = restart_game = false;
        rogue.nextGame = NG_NOTHING;
        rogue.nextGamePath[0] = '\0';
        rogue.nextGameSeed = 0;
        rogueMain();
        destroy_assets();
    } while (settings_changed || restart_game);
}

boolean resume() {
    if (resumed) {
        resumed = false;
        destroy_assets();
        create_assets();
        refreshScreen();
        return true;
    }
    return false;
}

boolean TouchScreenPauseForMilliseconds(short milliseconds, PauseBehavior behavior) {
    (void)behavior;  // Unused for now
    uint32_t init_time = SDL_GetTicks();
    draw_screen();
    uint32_t epoch = SDL_GetTicks() - init_time;
    if (epoch < milliseconds) {
        SDL_Delay(milliseconds - epoch);
    }
    resume();
    return process_events();
}

void TouchScreenNextKeyOrMouseEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance) {
    resume();
    while (!process_events()) {
        refresh_animations(colorsDance);
        TouchScreenPauseForMilliseconds(FRAME_INTERVAL, PAUSE_BEHAVIOR_DEFAULT);
    }
    *returnEvent = current_event;
    current_event.eventType = EVENT_ERROR;
}

void TouchScreenPlotChar(enum displayGlyph ch, short xLoc, short yLoc,
                         short foreRed, short foreGreen, short foreBlue,
                         short backRed, short backGreen, short backBlue) {
    SDL_FRect rect;
    rect.x = xLoc * cell_w;
    rect.y = yLoc * cell_h;
    rect.w = cell_w;
    rect.h = cell_h;
    SDL_SetRenderDrawColor(renderer, convert_color(backRed), convert_color(backGreen),
                           convert_color(backBlue), COLOR_MAX);
    SDL_RenderFillRectF(renderer, &rect);
    draw_glyph(ch, rect, convert_color(foreRed), convert_color(foreGreen), convert_color(foreBlue));
    screen_changed = true;
}

void TouchScreenRemap(const char *input_name, const char *output_name) {}

boolean TouchScreenModifierHeld(int modifier) {
    return modifier == 1 && ctrl_pressed;
}

static enum graphicsModes TouchScreenSetGraphicsMode(enum graphicsModes mode) {
    graphicsMode = mode;
    refreshScreen();
    return mode;
}

void TouchScreenTextInputStart() {
    if (!virtual_keyboard_active) {
        requires_text_input = true;
        start_text_input();
    }
}

void TouchScreenTextInputStop() {
    if (requires_text_input) {
        requires_text_input = false;
        stop_text_input();
    }
}

struct brogueConsole TouchScreenConsole = {
    .gameLoop = TouchScreenGameLoop,
    .pauseForMilliseconds = TouchScreenPauseForMilliseconds,
    .nextKeyOrMouseEvent = TouchScreenNextKeyOrMouseEvent,
    .plotChar = TouchScreenPlotChar,
    .remap = TouchScreenRemap,
    .modifierHeld = TouchScreenModifierHeld,
    .setGraphicsMode = TouchScreenSetGraphicsMode,
    // textInputStart and textInputStop not in brogueConsole struct
};

void brogue_main() {
    currentConsole = TouchScreenConsole;
    rogue.nextGame = NG_NOTHING;
    rogue.nextGamePath[0] = '\0';
    rogue.nextGameSeed = 0;
    currentConsole.gameLoop();
}

boolean serverMode = false;

// iOS-specific: Get the Documents directory path
char* get_documents_path() {
    static char path[PATH_MAX];
    // SDL provides platform-specific preference path
    char *pref_path = SDL_GetPrefPath("BrogueCE", "Brogue");
    if (pref_path) {
        strncpy(path, pref_path, PATH_MAX - 1);
        path[PATH_MAX - 1] = '\0';
        SDL_free(pref_path);
        return path;
    }
    // Fallback
    return ".";
}

int main(int argc, char *argv[]) {
    // Initialize SDL first to get proper paths
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        return -1;
    }

    // Change to the app's resource directory for assets
    char *base_path = SDL_GetBasePath();
    if (base_path) {
        chdir(base_path);
        SDL_free(base_path);
    }

    // Set up save directory in Documents
    char *save_path = get_documents_path();

    // Initialize defaults
    set_conf("", "");

    // Try to load config from save directory
    char config_path[PATH_MAX];
    snprintf(config_path, PATH_MAX, "%s/%s", save_path, SETTINGS_FILE);

    // Create version-specific save folder
    char save_folder[PATH_MAX];
    snprintf(save_folder, PATH_MAX, "%s/CE-%d.%d", save_path, BROGUE_MAJOR, BROGUE_MINOR);
    mkdir(save_folder, 0770);
    chdir(save_folder);

    // Load config
    load_conf();

    // Set graphics mode from config
    switch (default_graphics_mode) {
    case 0:
        graphicsMode = TEXT_GRAPHICS;
        break;
    case 1:
        graphicsMode = TILES_GRAPHICS;
        break;
    case 2:
        graphicsMode = HYBRID_GRAPHICS;
        break;
    }

    // Full SDL initialization
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL Error",
                                 "Unable to initialize SDL", NULL);
        return -1;
    }

    if (TTF_Init() != 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_ttf Error",
                                 "Unable to initialize SDL_ttf", NULL);
        SDL_Quit();
        return -1;
    }

    if (SDL_GetDisplayBounds(0, &screen) != 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL Error",
                                 "SDL_GetDisplayBounds failed", NULL);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    // Disable synthetic touch events from mouse - handle mouse directly
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetEventFilter(suspend_resume_filter, NULL);

    brogue_main();

    destroy_font();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
