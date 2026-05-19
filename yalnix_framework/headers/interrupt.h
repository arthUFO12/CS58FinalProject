#include "hardware.h"
#include "pcb.h"

void init_interrupt_vector(void);

void FullContextSwitch(pcb_t* curr_proc, pcb_t* next_proc);
