#include "syscalls.h"
#include "hardware.h"
#include "interrupt.h"
#include "kernel_memory.h"
#include "load_program.h"
#include "pcb.h"
#include "scheduler.h"
#include "tty.h"
#include "user_memory.h"
#include "ykernel.h"
#include "ylib.h"
#include "synchronization.h"
#include "pipe.h"

#define MAX_STRING_LEN 1024
#define MAX_ARGV_LEN 1024

static bool check_c_string(char* str) {
  if (str == NULL) return false;
  
  for (int i = 0; i < MAX_STRING_LEN; i++) {
    if (!has_perms((int) (long) (str + i), false)) return false;
    if (str[i] == '\0') return true;
  }

  return false;
}

static bool check_argv(char** argv) {
  if (argv == NULL) return false;

  for (int i = 0; i < MAX_ARGV_LEN; i++) {
    if (!has_perms((int) (long) (argv + i), false)) return false;
    if (argv[i] == NULL) return true;
  }

  return false;
}

static bool check_c_buf(char* str, int len, bool needs_write) {
  if (str == NULL) return false;
  
  for (int i = 0; i < len; i++) {
    if (!has_perms((int) (long) str + i, needs_write)) return false;
  }

  return true;
}

/* Return the PID of the currently running process. */
void KernelGetPid(UserContext *uc) { uc->regs[0] = get_running_proc()->pid; }

/* Adjust the heap break for the current process. */
void KernelBrk(UserContext *uc) { uc->regs[0] = KernelBrk_Impl((void *)uc->regs[0], uc->sp); }

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
  
  if (!check_c_string(filename)) goto string_error;
  if (!check_argv(argvec)) goto string_error;

  for (int i = 0; argvec[i] != NULL; i++) {
    if (!check_c_string(argvec[i])) goto string_error;
  }
  
  pcb_t *running_proc = get_running_proc();

  int res = LoadProgram(filename, argvec, running_proc);
  if (res == KILL) {
    KernelExit(NULL);
  } else if (res == ERROR) {
    uc->regs[0] = ERROR;
  }

  return;

string_error:
  TracePrintf(0, "Exec failure due to c string or argvec issues\n");
  uc->regs[0] = ERROR;
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
    if (uc->regs[0]) (*((int*) uc->regs[0])) = uc->regs[1];
  }
  uc->regs[0] = 0;
}

/* Terminal */
void KernelTtyRead(UserContext *uc) { 
  if (check_c_buf((char *)uc->regs[1], uc->regs[2], true)) kernel_tty_read(uc); 
  else {
    TracePrintf(0, "C buffer error. not all addresses in the region are readable and writable.\n");
    uc->regs[0] = ERROR;
    return;
  }
}

void KernelTtyWrite(UserContext *uc) { 
  if (check_c_buf((char *)uc->regs[1], uc->regs[2], false)) kernel_tty_write(uc); 
  else {
    TracePrintf(0, "C buffer error. not all addresses in the region are readable.\n");

    uc->regs[0] = ERROR;
    return;
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

void KernelSemInit(UserContext *uc) {
  SemInit_Impl(uc);
}

void KernelSemUp(UserContext *uc) {
  SemUp_Impl(uc);
}

void KernelSemDown(UserContext *uc) {
  SemDown_Impl(uc);
}

void KernelPipeInit(UserContext *uc) {
  PipeInit_Impl(uc);
}

void KernelPipeRead(UserContext *uc) {
  if (check_c_buf((char *)uc->regs[1], uc->regs[2], true)) PipeRead_Impl(uc);
  else {
    TracePrintf(0, "C buffer error. not all addresses in the region are readable and writable.\n");

    uc->regs[0] = ERROR;
    return;
  }
}

void KernelPipeWrite(UserContext *uc) {
  if (check_c_buf((char *)uc->regs[1], uc->regs[2], false)) PipeWrite_Impl(uc);
  else {
    TracePrintf(0, "C buffer error. not all addresses in the region are readable.\n");
    uc->regs[0] = ERROR;
    return;
  }
}
