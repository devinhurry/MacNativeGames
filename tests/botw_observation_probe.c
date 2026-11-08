/* Exercise loader dispatch and production ARM64 stack switches without game files.
   The recursive observer scenario follows nxnative t_gate_reentry.c (81880286, MIT). */
#define main runtime_loader_main
#include "../games/totk/runtime/loader.c"
#undef main
#include <assert.h>
#include <sys/wait.h>
#include "../games/totk/runtime/observation.h"
extern uint64_t runtime_hle_tsd_off;
extern uint64_t test_gate_observe(uint64_t value), test_gate_svc(uint64_t value);
extern const char test_gate_observe_ret[], test_gate_svc_ret[];
static _Thread_local unsigned observations, calls;
static char table_path[4096];
void runtime_game_data_path(const char *name, char *out, size_t cap) {
    assert(!strcmp(name,"runtime-tables.bin")); assert(snprintf(out,cap,"%s",table_path)<(int)cap);
}
static uint64_t host_double(uint64_t x) { return x * 2; }
void runtime_thread_note_hle(unsigned i) { (void)i; }
uint32_t runtime_thread_current_handle(void) { return 0; }
void runtime_spin_wait(unsigned i, uint64_t o) { (void)i; (void)o; assert(!"unexpected spin gate"); }
uint64_t runtime_svc(unsigned n, uint64_t *regs, const char *name, uint64_t off) {
    assert(n == 1 && !strcmp(name, "main") && off == 0x9000 && regs[0] >= 0x100);
    calls++; regs[17] = 0x1234; return regs[0] += 0x40;
}
static void change(uint64_t *regs, int param) {
    assert(param == 1 && regs[17] == 0x1717); regs[17] = 0x1234;
}
static void observe(const uint64_t *regs) {
    uint64_t saved[100]; memcpy(saved, regs, sizeof saved);
    assert(((uintptr_t)regs & 15) == 0 && (regs[99] & 15) == 0);
    assert(*(uint64_t *)(uintptr_t)regs[99] == regs[29]);
    assert(regs[3] == 0x3333 && regs[9] == 0x9999 && regs[16] == 0x1616 && regs[17] == 0x1234);
    assert(regs[32] == 0x5a5a5a5a5a5a5a5aULL && regs[95] == 0xa5a5a5a5a5a5a5a5ULL);
    assert(!pthread_getspecific(runtime_keys[4]));
    observations++;
    assert(test_gate_svc(0x100 + regs[0]) == 0x140 + regs[0]);
    if (regs[0] < 3) assert(test_gate_observe(regs[0] + 1) == 0xabc1 + regs[0]);
    uint64_t (*wrapped)(uint64_t) = runtime_hle_wrap((void *)host_double);
    assert(wrapped(0x123) == 0x246);   /* import made while the observation's host stack is already active */
    assert(!memcmp(saved, regs, sizeof saved));
}
static uint64_t result(const uint64_t *regs) { return 0xabc0 + regs[0]; }
static void *run(void *unused) {
    (void)unused;
    const size_t size = 2 * 1024 * 1024;
    void *stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    assert(stack != MAP_FAILED);
    uintptr_t top = (uintptr_t)stack + size;
    runtime_hle_stack_set(top);
    for (unsigned i = 0; i < 20; i++) {
        assert(test_gate_observe(0) == 0xabc0);
        assert((uintptr_t)pthread_getspecific(runtime_keys[4]) == top);
    }
    assert(observations == 80 && calls == 80);
    assert(!pthread_setspecific(runtime_keys[4], NULL) && !munmap(stack, size));
    return NULL;
}
static void check_table(int kind, unsigned build_id, int bad_hex, int success) {
    const char ids[]="\0cd57b23fa4bbad65803d9788c01821ee";
    TablesHeader header={.nmodule=2,.format=1,.strings=sizeof(TablesHeader)+2*sizeof(TablesModule)+sizeof(Gate),
                         .strings_size=sizeof ids,.size=sizeof(TablesHeader)+2*sizeof(TablesModule)+sizeof(Gate)+sizeof ids};
    memcpy(header.magic,"TOTKTAB1",8);strcpy(header.version,TOTK_GAME_DIR);
    TablesModule tables[2]={0};
    strcpy(tables[0].name,"main");strcpy(tables[1].name,"sdk");
    for(unsigned i=0;i<2;i++)snprintf(tables[i].digest,sizeof tables[i].digest,"%064d",0);
    tables[0].gates=sizeof header+sizeof tables;tables[0].ngate=1;tables[0].build_id=build_id;
    Gate gate={.ret=0x100,.timer=(unsigned)kind,.arg=OBS_BOTW_TOUCH_PANE};
    strcpy(table_path,"/tmp/botw-observation-table-XXXXXX");int fd=mkstemp(table_path);assert(fd>=0);
    FILE *out=fdopen(fd,"wb");assert(out);
    assert(fwrite(&header,1,sizeof header,out)==sizeof header && fwrite(tables,1,sizeof tables,out)==sizeof tables &&
           fwrite(&gate,1,sizeof gate,out)==sizeof gate && fwrite(ids,1,sizeof ids,out)==sizeof ids && !fclose(out));
    if(bad_hex){out=fopen(table_path,"r+b");assert(out && !fseek(out,(long)header.strings+1,SEEK_SET));fputc('x',out);assert(!fclose(out));}
    pid_t pid=fork();assert(pid>=0);
    if(!pid){
        load_tables();
        assert(!strcmp(modules[0].build_id,build_id?ids+1:""));
        assert(!strcmp(modules[1].build_id,""));_exit(0);
    }
    int status;assert(waitpid(pid,&status,0)==pid && WIFEXITED(status));
    assert((WEXITSTATUS(status)==0)==success);assert(!unlink(table_path));
}
int main(void) {
    assert(!pthread_key_create(&runtime_keys[4], NULL));
    runtime_hle_stack_init(runtime_keys[4] * 8);
    static Gate gates[2];
    gates[0] = (Gate){.ret=(uintptr_t)test_gate_observe_ret-1, .timer=3, .arg=OBS_BOTW_TOUCH_PANE | 1 << 8};
    gates[1] = (Gate){.ret=(uintptr_t)test_gate_svc_ret-1, .timer=0, .arg=1, .original=0x9000};
    qsort(gates, 2, sizeof *gates, gate_order);
    module_count=1; modules[0]=(Module){.name="main", .base=(void *)1, .gates=gates, .ngate=2};
    runtime_observation_register(OBS_BOTW_TOUCH_PANE | 1 << 8);
    assert(gate_obs_present(OBS_BOTW_TOUCH_PANE) && !gate_obs_present(OBS_BOTW_LOOK_VECTOR));
    gate_obs_set(OBS_BOTW_TOUCH_PANE, change);
    gate_obs_read_set(OBS_BOTW_TOUCH_PANE, observe);
    gate_obs_result_set(OBS_BOTW_TOUCH_PANE, result);
    pthread_t a, b;
    assert(!pthread_create(&a, NULL, run, NULL) && !pthread_create(&b, NULL, run, NULL));
    assert(!pthread_join(a, NULL) && !pthread_join(b, NULL));
    uint64_t regs[100], before[100];
    for (unsigned i=0; i<100; i++) regs[i]=i+100;
    memcpy(before, regs, sizeof regs);
    runtime_observation_dispatch(OBS_BOTW_LOOK_VECTOR, regs);  /* no callback means exact pass-through */
    assert(!memcmp(regs, before, sizeof regs));
    check_table(3,1,0,1);   /* preserved NSO build ID and observation kind */
    check_table(1,0,0,1);   /* old tables still have zero in the formerly reserved word */
    check_table(4,1,0,0);   /* an unknown kind must fail, never fall through to a timer */
    check_table(3,33,0,0);  /* truncated build ID */
    check_table(3,1,1,0);   /* nonhex build ID */
    puts("BOTW observations: callback ordering, parameters, x0 filtering, nested guest/SVC/import calls and per-thread frames passed");
}
