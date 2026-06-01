
#include "frame_tracking.h"
#include "hardware.h"
#include "interrupt.h"
#include "kernel_memory.h"
#include "load_program.h"
#include "pcb.h"
#include "scheduler.h"
#include "user_memory.h"
#include "ykernel.h"
#include "ylib.h"
#include "synchronization.h"
#include <assert.h>

/*
 * Kernel startup and bootstrap logic.
 * This module initializes memory, interrupts, scheduling, and loads
 * the first user-level program.
 */

static void do_idle(void);

static void do_idle(void) {
  while (1) {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}



static void set_up_uc(UserContext *uc, void (*idle_func)(void), void *sp) {
  uc->pc = (void *)idle_func;
  uc->sp = sp;
}

/*
 * KernelStart is called by the bootstrap code with the initial command line,
 * physical memory size, and a user context structure to populate for the
 * first process switch.
 */
void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt) {
  TracePrintf(0, "Initializing kernel\n");

  initialize_frame_tracking(pmem_size);
  init_kernel_brk();
  init_interrupt_vector();

  pcb_t *idle_pcb = create_new_pcb(K_STACK_VPNS, REGION1_VPNS, uctxt);
  pcb_t *init_pcb = create_new_pcb(K_STACK_VPNS, REGION1_VPNS, uctxt);

  init_region0_pt(idle_pcb);
  init_region1_pt(idle_pcb);

  init_sync();

  init_scheduler(idle_pcb);
  schedule_process(init_pcb);

  enable_vm();

  KernelContextSwitch(KCCopy, init_pcb, NULL);

  pcb_t *curr_proc = get_running_proc();

  char* init_name = cmd_args[0];

  if (curr_proc == init_pcb) {
    if (LoadProgram(init_name, cmd_args, curr_proc) != SUCCESS)
      Halt();

    memcpy(uctxt, &(curr_proc->uc), sizeof(UserContext));
  } else {
    set_up_uc(uctxt, do_idle, (void *)(VMEM_1_LIMIT - 1));
  }
}
