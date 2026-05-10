#include "hardware.h"
#include "scheduler.h"
#include "user_memory.h"
#include "ylib.h"
#include "interrupt.h"



void KernelGetPid(UserContext* uc) {

  uc->regs[0] = get_running_proc()->pid;
}

void KernelBrk(UserContext* uc) {
  uc->regs[0] = KernelBrk_Impl(uc->addr, uc->sp);
}

void KernelDelay(UserContext* uc) {
  int clock_ticks = uc->regs[0];
  pcb_t* running_proc = get_running_proc();

  
  if (clock_ticks <= 0) {
    uc->regs[0] = (clock_ticks == 0) ? 0 : ERROR;
  }

  pcb_t* new_proc = put_to_sleep(running_proc, clock_ticks);

  if (new_proc != running_proc) {
    uc->regs[0] = 0;
    FullContextSwitch(uc, running_proc, new_proc);
  }
  else {
    uc->regs[0] = ERROR;
  }

}