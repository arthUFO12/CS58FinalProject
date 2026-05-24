#pragma once
#include "bool.h"
#include "pcb.h"

/*
 * Scheduler API for Yalnix process management.
 * The scheduler maintains ready queues, sleeping processes, and blocked waiters.
 */

/* Place a process on the main ready queue. */
void schedule_process(pcb_t *new_proc);

/* Block a process waiting for a child to exit. */
void wait_block_process(pcb_t *new_proc);

/* Get the next process that should run, or the idle process if none are ready. */
pcb_t *get_next_process(void);

/* Return the currently running process. */
pcb_t *get_running_proc(void);

/* Put the current process to sleep for a fixed number of clock ticks. */
bool put_to_sleep(pcb_t *proc, int t);

/* Wake up any processes whose sleep timeout has expired. */
void wake_sleepers(void);

/* Wake any processes waiting for child termination. */
void wake_waiters(void);

/* Update the currently running process pointer. */
void set_running_proc(pcb_t *new_proc);

/* Advance the global clock tick counter. */
void increment_ticks(void);

/* Initialize scheduler state and install the idle process. */
bool init_scheduler(pcb_t *idle_proc);
