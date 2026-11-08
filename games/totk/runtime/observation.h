/* BOTW touch observation contract from nxnative 81880286 (MIT).
   IDs and callback ordering match the original host; stack ownership stays with this runtime's gate.S. */
#pragma once
#include <stdint.h>
enum {
    OBS_ASPECT_PROJECTION = 29, OBS_ASPECT_UI_CAMERA = 30, OBS_ASPECT_WORLD_PROJECTION = 31,
    OBS_BOTW_LOOK_VECTOR = 32, OBS_BOTW_LOOK_SOURCE = 33,
    OBS_BOTW_TOUCH_PANE = 34, OBS_BOTW_TOUCH_MATRIX = 35,
    OBS_BOTW_MENU_SELECT = 37, OBS_BOTW_LIST_WIDGET_UPDATE = 38,
    OBS_BOTW_UI_DRAW = 39, OBS_BOTW_UI_DRAW_END = 40, OBS_BOTW_SCREEN_DRAW = 41,
    OBS_BOTW_LOOK_PITCH = 42, OBS_BOTW_LOOK_YAW = 43, OBS_MAX = 44
};
void gate_obs_set(int id, void (*fn)(uint64_t *regs, int param));
void gate_obs_read_set(int id, void (*fn)(const uint64_t *regs));
void gate_obs_result_set(int id, uint64_t (*fn)(const uint64_t *regs));
int gate_obs_present(int id);
void runtime_observation_register(unsigned arg);
void runtime_observation_dispatch(unsigned arg, uint64_t *regs);
