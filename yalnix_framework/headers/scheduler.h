#include "bool.h"
#include "pcb.h"





void schedule_process(pcb_t* new_proc);

void block_process(pcb_t* new_proc);

pcb_t* run_diff_process(pcb_t* proc);

pcb_t* get_running_proc(void);

pcb_t* put_to_sleep(pcb_t* proc, int t);
void wake_sleepers(void);


void set_running_proc(pcb_t* new_proc);

void increment_ticks(void);

bool init_scheduler(pcb_t* idle_proc);