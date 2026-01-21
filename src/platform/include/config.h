#ifndef _config_h_
#define _config_h_

#include "Rogue.h"

#define SETTING_NAME_MAX_LEN 25
#define SETTING_VALUE_MAX_LEN 11
#define SETTINGS_FILE "settings.txt"

// Config Values - iOS defaults
extern double custom_cell_width;
extern double custom_cell_height;
extern int custom_screen_width;
extern int custom_screen_height;
extern boolean force_portrait;
extern boolean double_tap_lock;
extern int double_tap_interval;
extern boolean dynamic_colors;
extern boolean dpad_enabled;
extern int dpad_width;
extern int dpad_x_pos;
extern int dpad_y_pos;
extern boolean allow_dpad_mode_change;
extern boolean default_dpad_mode;
extern int long_press_interval;
extern int dpad_transparency;
extern int keyboard_visibility;
extern int zoom_mode;
extern double init_zoom;
extern boolean init_zoom_toggle;
extern double max_zoom;
extern boolean smart_zoom;
extern boolean left_panel_smart_zoom;
extern int filter_mode;
extern int default_graphics_mode;
extern boolean tiles_animation;
extern boolean blend_full_tiles;

extern boolean dpad_mode;
extern boolean restart_game;
extern boolean settings_changed;

void set_conf(const char *name, const char *value);
void load_conf();
void save_conf();
void init_default_config();

#endif
