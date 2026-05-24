
#include "frame_tracking.h"
#include "hardware.h"
#include "interrupt.h"
#include "kernel_memory.h"
#include "pcb.h"
#include "scheduler.h"
#include "user_memory.h"
#include "load_program.h"
#include "ylib.h"
#include <assert.h>

/*
 * Kernel startup logic for Yalnix.
 * This file initializes memory, interrupts, the scheduler, and the first user process.
 */

static void do_idle(void);

/* Idle loop executed when no user process is runnable. */
static void do_idle(void) {
  while (1) {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}

/* Helper to initialize a user context for the idle process. */
static void set_up_uc(UserContext* uc, void (*idle_func)(void), void* sp) {
  uc->pc = idle_func;
  uc->sp = sp;
}

/*
 * Kernel entry point called by the bootloader.
 * - cmd_args: command-line arguments for the initial process.
 * - pmem_size: size of physical memory.
 * - uctxt: initial user context area for the kernel to return into.
 */
void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt) {
  TracePrintf(0, "Initializing kernel\n");

  initialize_frame_tracking(pmem_size);
  init_kernel_brk();
  init_interrupt_vector();

  pcb_t* idle_pcb = create_new_pcb(NUM_K_STACK_VPNS, REGION1_VPNS, uctxt);
  pcb_t* init_pcb = create_new_pcb(NUM_K_STACK_VPNS, REGION1_VPNS, uctxt);

  init_region0_pt(idle_pcb);
  init_region1_pt(idle_pcb);

  init_scheduler(idle_pcb);
  schedule_process(init_pcb);
  enable_vm();
  KernelContextSwitch(KCCopy, init_pcb, NULL);

  pcb_t* curr_proc = get_running_proc();

  if (curr_proc == init_pcb) {
    LoadProgram(cmd_args[0], cmd_args + 1, curr_proc);
    memcpy(uctxt, &(curr_proc->uc), sizeof(UserContext));
  } else {
    set_up_uc(uctxt, do_idle, (void*) (VMEM_1_LIMIT - 1));
  }
}
