#include "bool.h"
#include "pcb.h"

void schedule_process(pcb_t *new_proc);

void wait_block_process(pcb_t *new_proc);

pcb_t *get_next_process(void);

pcb_t *get_running_proc(void);

pcb_t *put_to_sleep(pcb_t *proc, int t);

void wake_sleepers(void);

void wake_waiters();

void set_running_proc(pcb_t *new_proc);

void increment_ticks(void);

bool init_scheduler(pcb_t *idle_proc);
