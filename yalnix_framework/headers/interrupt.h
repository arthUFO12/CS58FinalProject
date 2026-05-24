#include "hardware.h"
#include "pcb.h"

/*
 * Initialize the interrupt vector table for Yalnix.
 * This installs trap handlers for the kernel and clock traps
 * and writes the vector base register.
 */
void init_interrupt_vector(void);

/*
 * Perform a full context switch from the current process to the next process.
 * - uc_in: pointer to the saved user context for the current trap frame.
 * - curr_proc: currently running process PCB.
 * - next_proc: PCB of the process to run next.
 */
void FullContextSwitch(UserContext* uc_in, pcb_t* curr_proc, pcb_t* next_proc);
