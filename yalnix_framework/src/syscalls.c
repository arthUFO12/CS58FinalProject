#include "hardware.h"
#include "scheduler.h"
#include "user_memory.h"
#include "ylib.h"
#include "interrupt.h"

/*
 * System call implementations for Yalnix.
 * These functions are invoked by the kernel trap handler.
 */

/* Return the current process PID to the user process. */
void KernelGetPid(UserContext* uc) {
  uc->regs[0] = get_running_proc()->pid;
}

/* Adjust the user heap break pointer and return status. */
void KernelBrk(UserContext* uc) {
  uc->regs[0] = KernelBrk_Impl(uc->regs[0], uc->sp);
}

/* Sleep the current process for the requested number of ticks. */
void KernelDelay(UserContext* uc) {
  int clock_ticks = uc->regs[0];
  pcb_t* running_proc = get_running_proc();

  if (clock_ticks <= 0) {
    uc->regs[0] = (clock_ticks == 0) ? 0 : ERROR;
    return;
  }

  pcb_t* new_proc = put_to_sleep(running_proc, clock_ticks);

  if (new_proc != running_proc) {
    uc->regs[0] = 0;
    FullContextSwitch(uc, running_proc, new_proc);
  } else {
    uc->regs[0] = ERROR;
  }
}
