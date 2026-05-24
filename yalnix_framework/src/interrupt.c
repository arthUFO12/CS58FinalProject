/**
 * @file interrupt.c
 * @brief Handles context switching and responding to traps
 */
#include "hardware.h"
#include "syscalls.h"
#include "yalnix.h"
#include "ylib.h"

#include "interrupt.h"
#include "kernel_memory.h"
#include "scheduler.h"
#include "syscalls.h"
#include "user_memory.h"

static void *interrupt_vector[TRAP_VECTOR_SIZE];

void FullContextSwitch(pcb_t *curr_proc, pcb_t *next_proc) {
  UCSwitch(next_proc);
  set_running_proc(next_proc);

  TracePrintf(0, "Switching to process %d to process %d\n", (curr_proc != NULL) ? curr_proc->pid : -1, next_proc->pid);
  int success = KernelContextSwitch(KCSwitch, curr_proc, next_proc);

  if (success == ERROR) {
    TracePrintf(0, "An error occurred. Context Switch Failed from proc %d to proc %d. Exiting.\n", curr_proc->pid,
                next_proc->pid);
    Halt();
  }
}

/* Trap Handlers */
static void trap_kernel_handler(UserContext *uc) {
  TracePrintf(1, "A trap kernel with code %x\n", uc->code);

  unsigned int code = (unsigned int)uc->code;
  pcb_t *curr_proc = get_running_proc();

  memcpy(&(curr_proc->uc), uc, sizeof(UserContext));

  if (code == YALNIX_GETPID) {
    KernelGetPid(&(curr_proc->uc));
  } else if (code == YALNIX_BRK) {
    KernelBrk(&(curr_proc->uc));
  } else if (code == YALNIX_DELAY) {
    KernelDelay(&(curr_proc->uc));
  } else if (code == YALNIX_EXIT) {
    KernelExit(&(curr_proc->uc));
  } else if (code == YALNIX_FORK) {
    KernelFork(&(curr_proc->uc));
  } else if (code == YALNIX_EXEC) {
    KernelExec(&(curr_proc->uc));
  } else if (code == YALNIX_WAIT) {
    KernelWait(&(curr_proc->uc));
  } else {
    TracePrintf(1, "Syscall not implemented\n");
    TracePrintf(1, "Address %p\n", uc->addr);
  }

  curr_proc = get_running_proc();

  memcpy(uc, &(curr_proc->uc), sizeof(UserContext));
}

static void trap_clock_handler(UserContext *uc) {
  wake_waiters();
  wake_sleepers();
  increment_ticks();
  TracePrintf(1, "A trap clock occurred. \n");

  pcb_t *curr_proc = get_running_proc();

  memcpy(&(curr_proc->uc), uc, sizeof(UserContext));
  schedule_process(curr_proc);

  pcb_t *next_proc = get_next_process();

  if (curr_proc != next_proc) {
    FullContextSwitch(curr_proc, next_proc);
  }

  curr_proc = get_running_proc();

  memcpy(uc, &(curr_proc->uc), sizeof(UserContext));
}

static void trap_illegal_handler(UserContext *uc) {
  /*
  save current ctxt into PCB

  abort_process();

  choose next ready process

  restore next process uctxct

  context switch
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

void init_interrupt_vector() {
  for (int i = 0; i < TRAP_VECTOR_SIZE; i++) {
    interrupt_vector[i] = (void *)trap_not_implemented;
  }
  interrupt_vector[TRAP_KERNEL] = (void *)trap_kernel_handler;
  interrupt_vector[TRAP_CLOCK] = (void *)trap_clock_handler;

  WriteRegister(REG_VECTOR_BASE, (unsigned int)(long)interrupt_vector);
}
