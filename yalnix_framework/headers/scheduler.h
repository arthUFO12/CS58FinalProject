#include "bool.h"
#include "pcb.h"

/*
 * Process scheduling API for Yalnix.
 * This module manages ready and blocked queues, sleeping processes,
 * and the currently running process.
 */





/* Place a process into the ready queue. */
void schedule_process(pcb_t* new_proc);

/* Block a process until an external event wakes it. */
void block_process(pcb_t* new_proc);

/* Select a different process to run next, if available. */
pcb_t* run_diff_process(pcb_t* proc);

/* Return the currently running process. */
pcb_t* get_running_proc(void);

/* Put the current process to sleep for t ticks and return the next process. */
pcb_t* put_to_sleep(pcb_t* proc, int t);

/* Wake up any sleeping processes whose timer has expired. */
void wake_sleepers(void);

/* Set the currently running process pointer. */
void set_running_proc(pcb_t* new_proc);

/* Increment the global tick counter used for sleeping and scheduling. */
void increment_ticks(void);

/* Initialize scheduler state with the idle process PCB. */
bool init_scheduler(pcb_t* idle_proc);