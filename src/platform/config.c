// BrogueCE iOS - Configuration
// Simplified from Android port - removes settings menu UI

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Config values with iOS-friendly defaults
double custom_cell_width = 0;
double custom_cell_height = 0;
int custom_screen_width = 0;
int custom_screen_height = 0;
boolean force_portrait = false;
boolean double_tap_lock = true;
int double_tap_interval = 500;
boolean dynamic_colors = true;
boolean dpad_enabled = true;
int dpad_width = 0;
int dpad_x_pos = 0;
int dpad_y_pos = 0;
boolean allow_dpad_mode_change = true;
boolean default_dpad_mode = true;  // Movement mode by default
int long_press_interval = 750;
int dpad_transparency = 75;
int keyboard_visibility = 1;  // On-demand
int zoom_mode = 1;  // Follow player
double init_zoom = 2.0;
boolean init_zoom_toggle = false;
double max_zoom = 4.0;
boolean smart_zoom = true;
boolean left_panel_smart_zoom = true;
int filter_mode = 2;  // Anisotropic
int default_graphics_mode = 1;  // Tiles by default on iOS
boolean tiles_animation = true;
boolean blend_full_tiles = true;

boolean dpad_mode = true;  // Start in movement mode
boolean restart_game = false;
boolean settings_changed = false;

void init_default_config() {
    // Set iOS-optimized defaults
    dpad_enabled = true;
    default_graphics_mode = 1;  // Tiles look better on mobile
    zoom_mode = 1;
    init_zoom = 2.0;
    max_zoom = 4.0;
    smart_zoom = true;
    dpad_mode = default_dpad_mode;
}

void set_conf(const char *name, const char *value) {
    if (strlen(name) == 0) {
        init_default_config();
        return;
    }

    // Parse known settings
    if (strcmp(name, "custom_cell_width") == 0) {
        custom_cell_width = atof(value);
    } else if (strcmp(name, "custom_cell_height") == 0) {
        custom_cell_height = atof(value);
    } else if (strcmp(name, "force_portrait") == 0) {
        force_portrait = atoi(value);
    } else if (strcmp(name, "dynamic_colors") == 0) {
        dynamic_colors = atoi(value);
    } else if (strcmp(name, "dpad_enabled") == 0) {
        dpad_enabled = atoi(value);
    } else if (strcmp(name, "dpad_width") == 0) {
        dpad_width = atoi(value);
    } else if (strcmp(name, "dpad_transparency") == 0) {
        dpad_transparency = atoi(value);
    } else if (strcmp(name, "default_dpad_mode") == 0) {
        default_dpad_mode = atoi(value);
        dpad_mode = default_dpad_mode;
    } else if (strcmp(name, "zoom_mode") == 0) {
        zoom_mode = atoi(value);
    } else if (strcmp(name, "init_zoom") == 0) {
        init_zoom = atof(value);
    } else if (strcmp(name, "max_zoom") == 0) {
        max_zoom = atof(value);
    } else if (strcmp(name, "default_graphics_mode") == 0) {
        default_graphics_mode = atoi(value);
    } else if (strcmp(name, "tiles_animation") == 0) {
        tiles_animation = atoi(value);
    } else if (strcmp(name, "filter_mode") == 0) {
        filter_mode = atoi(value);
    } else if (strcmp(name, "double_tap_lock") == 0) {
        double_tap_lock = atoi(value);
    } else if (strcmp(name, "double_tap_interval") == 0) {
        double_tap_interval = atoi(value);
    } else if (strcmp(name, "long_press_interval") == 0) {
        long_press_interval = atoi(value);
    } else if (strcmp(name, "smart_zoom") == 0) {
        smart_zoom = atoi(value);
    }
}

void load_conf() {
    FILE *f = fopen(SETTINGS_FILE, "r");
    if (f == NULL) {
        return;
    }

    char name[SETTING_NAME_MAX_LEN];
    char value[SETTING_VALUE_MAX_LEN];

    while (fscanf(f, "%24s %10s", name, value) == 2) {
        set_conf(name, value);
    }

    fclose(f);
}

void save_conf() {
    FILE *f = fopen(SETTINGS_FILE, "w");
    if (f == NULL) {
        return;
    }

    fprintf(f, "dpad_enabled %d\n", dpad_enabled);
    fprintf(f, "default_dpad_mode %d\n", default_dpad_mode);
    fprintf(f, "dpad_transparency %d\n", dpad_transparency);
    fprintf(f, "zoom_mode %d\n", zoom_mode);
    fprintf(f, "init_zoom %.1f\n", init_zoom);
    fprintf(f, "max_zoom %.1f\n", max_zoom);
    fprintf(f, "default_graphics_mode %d\n", default_graphics_mode);
    fprintf(f, "tiles_animation %d\n", tiles_animation);
    fprintf(f, "dynamic_colors %d\n", dynamic_colors);
    fprintf(f, "smart_zoom %d\n", smart_zoom);
    fprintf(f, "filter_mode %d\n", filter_mode);

    fclose(f);
}
