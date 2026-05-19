/**
 * @file syscalls.c
 * @brief Define system calls
 */
#include "hardware.h"
#include "yalnix.h"
#include "ykernel.h"
#include "ylib.h"

#include "hardware.h"
#include "interrupt.h"
#include "kernel_memory.h"
#include "scheduler.h"
#include "syscalls.h"
#include "user_memory.h"

void KernelGetPid(UserContext *uc) { uc->regs[0] = get_running_proc()->pid; }

void KernelBrk(UserContext *uc) { uc->regs[0] = KernelBrk_Impl(uc->addr, uc->sp); }

void KernelDelay(UserContext *uc) {
  int clock_ticks = uc->regs[0];
  pcb_t *running_proc = get_running_proc();

  if (clock_ticks <= 0) {
    uc->regs[0] = (clock_ticks == 0) ? 0 : ERROR;
  }

  pcb_t *new_proc = put_to_sleep(running_proc, clock_ticks);

  if (new_proc != running_proc) {
    uc->regs[0] = 0;
    FullContextSwitch(uc, running_proc, new_proc);
  } else {
    uc->regs[0] = ERROR;
  }
}

void KernelFork(UserContext *uc) {

  pcb_t *parent_pcb = get_running_proc();

  pcb_t *child_pcb = create_new_pcb(NUM_K_STACK_VPNS, REGION1_VPNS, uc);

  // copy usercontext, duplicate page table, copy pages
  UCCopy(&(parent_pcb->uc), child_pcb, NULL);

  child_pcb->pid = helper_new_pid(child_pcb->mem_ctx.region1_pt);
  child_pcb->state = READY;

  schedule_process(child_pcb);

  // ucopy sets child_pcb->uc.regs[0] to 0 already
  parent_pcb->uc.regs[0] = child_pcb->pid;
}

void KernelExec(UserContext *uc) {
  /*
  get filename and args ( regs[0] and regs[1])

  validate pointers in user memory

  LoadProgram()

  */
}

void KernelExit(UserContext *uc) {
  /*
   current_pcb = get_running_proc();

   Set status to exit

   wake parent if necessary

   undo_allocation()

   mark as zombie

   next = run_diff_process(current)

   FillContextSwitch(uc, current, next);
   */
}

void KernelWait(UserContext *uc) {
  /*
  parent_pcb = get_running_proc();

  Check if no children exist: return error

  If zombie exists reap: return pid

  Block parent

  */
}

void KernelTtyRead(UserContext *uc) {
  /*
    If no data: block

    Copy data into user buffer
    */
}

void KernelTtyWrite(UserContext *uc) {
  /*
  Queue write request

  If tty idle: start transmit

  Block until transmit ends
  */
}
