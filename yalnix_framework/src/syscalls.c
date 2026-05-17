/**
 * @file syscalls.c
 * @brief Define system calls
 */
#include "interrupt.h"
#include "scheduler.h"
#include "user_memory.h"
#include "ylib.h"

#include "syscalls.h"

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
  /*
  parent_pcb = get_running_proc()

  new_pcb = create_new_pcb()

  //copy usercontext, duplicate page table, copy pages
   UCopy()

  add child to calling process

  return 0 to child (regs[0] = 0)
  return child pid to parent
  */
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
