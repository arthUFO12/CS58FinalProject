#include "interrupt.h"

#include "user_memory.h"
#include "kernel_memory.h"
#include "scheduler.h"
#include "ylib.h"


void FullContextSwitch(UserContext* uc_in, pcb_t* curr_proc, pcb_t* next_proc) {
  UCSwitch(uc_in, curr_proc, next_proc);
  set_running_proc(next_proc);

  TracePrintf(0, "Switching to process %d to process %d\n", curr_proc->pid, next_proc->pid);
  int success = KernelContextSwitch(KCSwitch, curr_proc, next_proc);

  if (success == -1) {
    TracePrintf(0, "An error occurred. Context Switch Failed from proc %d to proc %d. Exiting.\n", curr_proc->pid, next_proc->pid);
    Halt();
  }
}
static void trap_kernel_handler(UserContext *uc) {
  TracePrintf(1, "A trap kernel with code %x\n", uc->code);
}

static void trap_clock_handler(UserContext *uc) {
  increment_ticks();
  TracePrintf(1, "A trap clock occurred. \n");


  pcb_t* curr_proc = get_running_proc();
  pcb_t* next_proc = run_diff_process(curr_proc);

  memcpy(&(curr_proc->uc), uc, sizeof(UserContext));

  if (curr_proc != next_proc) {
    FullContextSwitch(uc, curr_proc, next_proc);
  }

  curr_proc = get_running_proc();
  
  memcpy(uc, &(curr_proc->uc), sizeof(UserContext));
}

static void trap_not_implemented(UserContext *uc) {
  TracePrintf(1, "A trap that isn't implemented occurred\n");
}

static void *interrupt_vector[TRAP_VECTOR_SIZE];

void init_interrupt_vector() {
  for (int i = 0; i < TRAP_VECTOR_SIZE; i++) {
    interrupt_vector[i] = (void*) trap_not_implemented;
  }
  interrupt_vector[TRAP_KERNEL] = (void*) trap_kernel_handler;
  interrupt_vector[TRAP_CLOCK] = (void*) trap_clock_handler;

  WriteRegister(REG_VECTOR_BASE, (unsigned int) (long) interrupt_vector);
}
