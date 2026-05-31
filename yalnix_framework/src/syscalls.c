#include "syscalls.h"
#include "interrupt.h"
#include "kernel_memory.h"
#include "load_program.h"
#include "pcb.h"
#include "scheduler.h"
#include "user_memory.h"
#include "ykernel.h"
#include "ylib.h"
#include "synchronization.h"

/* Return the PID of the currently running process. */
void KernelGetPid(UserContext *uc) {
  uc->regs[0] = get_running_proc()->pid;
}

/* Adjust the heap break for the current process. */
void KernelBrk(UserContext *uc) {
  uc->regs[0] = KernelBrk_Impl((void *)uc->regs[0], uc->sp);
}

/* Sleep the current process for a given number of clock ticks. */
void KernelDelay(UserContext *uc) {
  int clock_ticks = uc->regs[0];
  pcb_t *running_proc = get_running_proc();

  if (clock_ticks <= 0) {
    uc->regs[0] = (clock_ticks == 0) ? 0 : ERROR;
    return;
  }

  if (put_to_sleep(running_proc, clock_ticks)) {
    uc->regs[0] = 0;
    pcb_t *new_proc = get_next_process();
    FullContextSwitch(running_proc, new_proc);
  } else {
    uc->regs[0] = ERROR;
  }
}

/* Terminate the current process and switch to the next runnable process. */
void KernelExit(UserContext *uc) {
  pcb_t *running_proc = get_running_proc();

  if (running_proc->pid == 1)
    Halt();

  pcb_t *new_proc = get_next_process();
  running_proc->exit_code = (uc == NULL) ? -1 : (int)uc->regs[0];
  running_proc->state = EXITED;

  deallocate_region1();
  deallocate_kernel_stack();
  retire_pcb(running_proc);

  FullContextSwitch(NULL, new_proc);
}

/* Replace the current process image with a new executable. */
void KernelExec(UserContext *uc) {
  char *filename = (char *)uc->regs[0];
  char **argvec = (char **)uc->regs[1];
  pcb_t *running_proc = get_running_proc();

  int res = LoadProgram(filename, argvec, running_proc);
  if (res == KILL) {
    KernelExit(NULL);
  } else if (res == ERROR) {
    uc->regs[0] = ERROR;
  }
}

/* Create a new process by copying the current process state. */
void KernelFork(UserContext *uc) {
  pcb_t *running_proc = get_running_proc();
  pcb_t *new_proc = create_new_pcb(K_STACK_VPNS, REGION1_VPNS, uc);

  if (!UCCopy(uc, new_proc)) {
    uc->regs[0] = ERROR;
    retire_pcb(new_proc);
    free(new_proc);
    return;
  }

  uc->regs[0] = new_proc->pid;
  new_proc->uc.regs[0] = 0;
  add_child_proc(running_proc, new_proc);
  schedule_process(new_proc);
  KernelContextSwitch(KCCopy, new_proc, NULL);
}

/* Wait until one child process exits, blocking the current process if needed. */
void KernelWait(UserContext *uc) {
  pcb_t *running_proc = get_running_proc();
  if (!find_exited_child(running_proc, (int *)uc->regs[0])) {
    wait_block_process(running_proc);
    pcb_t *new_proc = get_next_process();
    FullContextSwitch(running_proc, new_proc);
  }
}

void KernelLockInit(UserContext* uc) {
  LockInit_Impl(uc);
}

void KernelAcquire(UserContext *uc) {
  Acquire_Impl(uc);
}

void KernelRelease(UserContext *uc) {
  Release_Impl(uc);
}

void KernelCvarInit(UserContext *uc) {
  CvarInit_Impl(uc);
}

void KernelCvarSignal(UserContext *uc) {
  CvarSignal_Impl(uc);
}

void KernelCvarBroadcast(UserContext *uc) {
  CvarBroadcast_Impl(uc);
}

void KernelCvarWait(UserContext *uc) {
  CvarWait_Impl(uc);
}

void KernelReclaim(UserContext *uc) {
  Reclaim_Impl(uc);
}
