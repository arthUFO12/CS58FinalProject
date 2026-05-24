#pragma once
#include "hardware.h"
#include "pcb.h"

/*
 * Interrupt vector initialization and full context switching interface.
 * This file exposes the kernel's trap handler setup and process switch
 * entry points.
 */
void init_interrupt_vector(void);

/*
 * Perform a full context switch from curr_proc to next_proc.
 * The current process context is saved and the next process is resumed.
 */
void FullContextSwitch(pcb_t *curr_proc, pcb_t *next_proc);
