/**
 * @file interrupt.c
 * @brief Handles context switching and responding to traps
 */
#include "interrupt.h"

#include "kernel_memory.h"
#include "scheduler.h"
#include "user_memory.h"
#include "ylib.h"

void FullContextSwitch(UserContext *uc_in, pcb_t *curr_proc, pcb_t *next_proc) {
  UCSwitch(uc_in, curr_proc, next_proc);
  set_running_proc(next_proc);

  TracePrintf(0, "Switching to process %d to process %d\n", curr_proc->pid, next_proc->pid);
  int success = KernelContextSwitch(KCSwitch, curr_proc, next_proc);

  if (success == ERROR) {
    TracePrintf(0, "An error occurred. Context Switch Failed from proc %d to proc %d. Exiting.\n", curr_proc->pid,
                next_proc->pid);
    Halt();
  }
}
/* Trap Handlers */
static void trap_kernel_handler(UserContext *uc) {
  /*

    save current ctxt into PCB

    extract sycall from uctxt code

    switch syscall:
      YALNIX_FORK;
      YALNIX_EXEC;
      YALNIX_EXIT;
      ...

    return value from syscall to userprocess through regs[0] in uctxt

    restore uctxt
   */
}

static void trap_clock_handler(UserContext *uc) {
  /*
  save current ctxt into PCB

  increment_ticks();

  choose next ready process

  context switch if necessary

  restore next process uctxct
  */
}

static void trap_illegal_handler(UserContext *uc) {
  /*
  save current ctxt into PCB

  abort_process();

  choose next ready process

  context switch

  restore next process uctxct
  */
}

static void trap_memory_handler(UserContext *uc) {
  /*
    if not memory enlarge request:
      abort_process();
      return

    enlarge process stack

    return to user process
   */
}

static void trap_math_handler(UserContext *uc) {
  /* (same as illegal)
  save current ctxt into PCB

  abort_process();

  choose next ready process

  context switch

  restore next process uctxct
   */
}
static void trap_tty_receive_handler(UserContext *uc) {
  /*
   Ttyrecieve();

   */
}
static void trap_tty_transmit_handler(UserContext *uc) {
  /*

   */
}
static void trap_disk_handler(UserContext *uc) {
  /*
   */
}

static void trap_not_implemented(UserContext *uc) { TracePrintf(1, "A trap that isn't implemented occurred\n"); }

static void *interrupt_vector[TRAP_VECTOR_SIZE];

void init_interrupt_vector() {
  for (int i = 0; i < TRAP_VECTOR_SIZE; i++) {
    interrupt_vector[i] = (void *)trap_not_implemented;
  }
  interrupt_vector[TRAP_KERNEL] = (void *)trap_kernel_handler;
  interrupt_vector[TRAP_CLOCK] = (void *)trap_clock_handler;

  WriteRegister(REG_VECTOR_BASE, (unsigned int)(long)interrupt_vector);
}
