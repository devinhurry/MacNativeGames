/* The new input adapter with the preserved feature implementations, no game data or platform UI. */
#include "../games/totk/runtime/botw_touch/bridge.h"
#include "../games/totk/runtime/botw_touch/front.h"
#include "../games/totk/runtime/botw_touch/dm_input.h"
#include "../games/totk/runtime/botw_touch/botw_touch_menu.h"
#include <assert.h>
static uint64_t frame;
static RuntimeGamepadState published;
static uint64_t cancelled;
static int cleared_window;
uint64_t dm_nvn_frame_count(void) { return frame; }
uint64_t runtime_now_ns(void) { return frame*16666667; }
_Noreturn void runtime_fail(const char *message) { fprintf(stderr,"%s\n",message);abort(); }
int runtime_guest_memory_contains(uintptr_t address) { (void)address;return 0; }
int runtime_handheld_mode(void) { return 1; }
void runtime_touch_pad_set(const RuntimeGamepadState *state) { published=*state; }
void runtime_touch_pad_cancel_latch(uint64_t buttons) { cancelled=buttons; }
void runtime_input_clear_window(void) { cleared_window++; }
void front_look_mode_changed(void) {}
void front_botw_touch_hud_mode_changed(void) {}
void front_botw_touch_hud_map_region(int v,float x,float y,float w,float h) { (void)v;(void)x;(void)y;(void)w;(void)h; }
void front_botw_touch_action_changed(void) {}
void front_botw_rune_icon_changed(void) {}
void front_botw_touch_menu_changed(void) {}
void dm_look_test_vector_config(module_t *main,float gain);
int main(void) {
    dm_look_test_vector_config(NULL,60);
    RuntimeGamepadState physical={.buttons=DM_BTN_A,.rx=10000,.ry=5000};
    RuntimeGamepadState touch={.buttons=DM_BTN_B,.ry=22000};
    RuntimeGamepadState result={.rx=6000};
    frame=10;runtime_botw_touch_merge(&result,&physical,&touch);
    assert(result.buttons==(DM_BTN_A|DM_BTN_B) && result.rx==10000 && result.ry==5000);
    dm_hid_look_sample sample;
    assert(dm_hid_look_sample_at(10,&sample));
    assert(sample.source==DM_HID_LOOK_PHYSICAL && sample.virtual_rx==6000 && sample.virtual_ry==22000);
    assert(!dm_hid_look_sample_at(9,&sample));
    physical.rx=25000;physical.ry=-12000;result=(RuntimeGamepadState){0};
    runtime_botw_touch_merge(&result,&physical,&touch);
    assert(result.rx==10000 && result.ry==5000); /* all reads of one present use the same pair */
    frame=11;physical=(RuntimeGamepadState){.rx=100,.ry=200};touch=(RuntimeGamepadState){.rx=2000,.ry=6000};
    result=(RuntimeGamepadState){0};runtime_botw_touch_merge(&result,&physical,&touch);
    assert(result.rx==2000 && result.ry==6000);
    assert(dm_hid_look_sample_at(11,&sample) && sample.source==DM_HID_LOOK_VIRTUAL);
    frame=12;dm_look_input(DM_LOOK_BEGIN,0,0);dm_look_input(DM_LOOK_MOVE,.1f,.2f);
    physical=touch=result=(RuntimeGamepadState){0};runtime_botw_touch_merge(&result,&physical,&touch);
    assert(dm_hid_look_sample_at(12,&sample) && sample.source==DM_HID_LOOK_RELATIVE);
    frame=13;physical.rx=20000;result=(RuntimeGamepadState){0};runtime_botw_touch_merge(&result,&physical,&touch);
    assert(result.rx==20000 && result.ry==0);
    assert(dm_hid_look_sample_at(13,&sample) && sample.source==DM_HID_LOOK_PHYSICAL);
    dm_look_input(DM_LOOK_CANCEL,0,0);
    dm_input_set(DM_BTN_ZL,1234,-5678,9876,-4321);
    assert(published.connected && published.buttons==DM_BTN_ZL && published.lx==1234 && published.ry==-4321);
    dm_input_cancel_virtual_latch(DM_BTN_ZR);assert(cancelled==DM_BTN_ZR);
    front_clear_window_input();assert(cleared_window==1);
    /* The import adapter consumes the actual original short-tap latch. */
    dm_touch finger={.id=3,.x=.25f,.y=.5f};dm_touch_set(&finger,1);dm_touch_set(NULL,0);
    void (*read_touch)(void *)=runtime_botw_touch_override("_ZN2nn3hid19GetTouchScreenStateILm16EEEvPNS0_16TouchScreenStateIXT_EEE");
    assert(read_touch && !runtime_botw_touch_override("not an import"));
    uint8_t state[0x290];read_touch(state);int32_t count,x,y;uint32_t attr;
    memcpy(&count,state+8,4);memcpy(&attr,state+0x18,4);memcpy(&x,state+0x20,4);memcpy(&y,state+0x24,4);
    assert(count==1 && attr==1 && x==320 && y==360);
    read_touch(state);memcpy(&attr,state+0x18,4);assert(attr==2);
    read_touch(state);memcpy(&count,state+8,4);assert(count==0);
    puts("BOTW input adapter: vector priority, frozen source, relative ownership, virtual latch isolation and touchscreen imports passed");
}
