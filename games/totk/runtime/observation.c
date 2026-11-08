/* Callback registry and ordering adapted from nxnative dm/host/hle.c, 81880286 (MIT).
   Register at startup, before guest threads begin. Nested calls use the existing host stack (gate.S). */
#include "runtime.h"
#include "observation.h"
static void (*g_obs_fn[OBS_MAX])(uint64_t *regs, int param);
static void (*g_obs_read_fn[OBS_MAX])(const uint64_t *regs);
static uint64_t (*g_obs_result_fn[OBS_MAX])(const uint64_t *regs);
static int g_obs_have[OBS_MAX];
static void check_id(unsigned id) {
    if (!id || id >= OBS_MAX) runtime_fail("observation gate number out of range");
}
void gate_obs_set(int id, void (*fn)(uint64_t *regs, int param)) {
    check_id((unsigned)id); g_obs_fn[id] = fn;
}
void gate_obs_read_set(int id, void (*fn)(const uint64_t *regs)) {
    check_id((unsigned)id); g_obs_read_fn[id] = fn;
}
void gate_obs_result_set(int id, uint64_t (*fn)(const uint64_t *regs)) {
    check_id((unsigned)id); g_obs_result_fn[id] = fn;
}
int gate_obs_present(int id) { return id > 0 && id < OBS_MAX && g_obs_have[id]; }
void runtime_observation_register(unsigned arg) { check_id(arg & 0xff); g_obs_have[arg & 0xff] = 1; }
void runtime_observation_dispatch(unsigned arg, uint64_t *regs) {
    unsigned id = arg & 0xff; check_id(id);
    if (g_obs_fn[id]) g_obs_fn[id](regs, (int)(arg >> 8));
    if (g_obs_read_fn[id]) g_obs_read_fn[id](regs);
    if (g_obs_result_fn[id]) regs[0] = g_obs_result_fn[id](regs);
}
