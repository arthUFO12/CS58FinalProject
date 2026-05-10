#include "hardware.h"
#include "pcb.h"

void init_interrupt_vector(void);

void FullContextSwitch(UserContext* uc_in, pcb_t* curr_proc, pcb_t* next_proc);
