#include "interrupt.h"

#include "kernel_memory.h"
#include "scheduler.h"
#include "user_memory.h"
#include "ylib.h"
#include "yalnix.h"
#include "syscalls.h"

/*
 * Interrupt and trap handling for the Yalnix kernel.
 * This module dispatches traps and syscalls, and performs kernel
 * context switches when needed.
 */

/* Perform a full context switch between two processes. */
void FullContextSwitch(pcb_t *curr_proc, pcb_t *next_proc) {
  UCSwitch(next_proc);
  set_running_proc(next_proc);

  TracePrintf(3, "Switching to process %d to process %d\n", (curr_proc != NULL) ? curr_proc->pid : -1,
              next_proc->pid);
  int success = KernelContextSwitch(KCSwitch, curr_proc, next_proc);

  if (success == -1) {
    TracePrintf(0,
                "An error occurred. Context Switch Failed from proc %d to proc "
                "%d. Exiting.\n",
                curr_proc->pid, next_proc->pid);
    Halt();
  }
}


static void trap_kernel_handler(UserContext *uc);
static void trap_clock_handler(UserContext *uc);
static void trap_math_handler(UserContext *uc);
static void trap_illegal_handler(UserContext *uc);
static void trap_memory_handler(UserContext *uc);
static void trap_not_implemented(UserContext *uc);


static void *interrupt_vector[TRAP_VECTOR_SIZE];

/* Initialize the hardware trap vector to dispatch kernel handlers. */
void init_interrupt_vector() {
  for (int i = 0; i < TRAP_VECTOR_SIZE; i++) {
    interrupt_vector[i] = (void *)trap_not_implemented;
  }

  interrupt_vector[TRAP_KERNEL] = (void *)trap_kernel_handler;
  interrupt_vector[TRAP_CLOCK] = (void *)trap_clock_handler;
  interrupt_vector[TRAP_MATH] = (void *)trap_math_handler;
  interrupt_vector[TRAP_ILLEGAL] = (void *)trap_illegal_handler;
  interrupt_vector[TRAP_MEMORY] = (void *)trap_memory_handler;
  WriteRegister(REG_VECTOR_BASE, (unsigned int)(long)interrupt_vector);
}



/* Dispatch a kernel trap to the appropriate syscall handler. */
static void trap_kernel_handler(UserContext *uc) {
  TracePrintf(2, "A trap kernel with code %x\n", uc->code);

  unsigned int code = (unsigned int) uc->code;
  pcb_t *curr_proc = get_running_proc();

  memcpy(&(curr_proc->uc), uc, sizeof(UserContext));

  switch (code) {
    case YALNIX_GETPID:
      KernelGetPid(&(curr_proc->uc));
      break;

    case YALNIX_BRK:
      KernelBrk(&(curr_proc->uc));
      break;

    case YALNIX_DELAY:
      KernelDelay(&(curr_proc->uc));
      break;

    case YALNIX_EXIT:
      KernelExit(&(curr_proc->uc));
      break;

    case YALNIX_FORK:
      KernelFork(&(curr_proc->uc));
      break;

    case YALNIX_EXEC:
      KernelExec(&(curr_proc->uc));
      break;

    case YALNIX_WAIT:
      KernelWait(&(curr_proc->uc));
      break;

    case YALNIX_LOCK_INIT:
      KernelLockInit(&(curr_proc->uc));
      break;

    case YALNIX_CVAR_INIT:
      KernelCvarInit(&(curr_proc->uc));
      break;

    case YALNIX_LOCK_ACQUIRE:
      KernelAcquire(&(curr_proc->uc));
      break;

    case YALNIX_LOCK_RELEASE:
      KernelRelease(&(curr_proc->uc));
      break;

    case YALNIX_CVAR_SIGNAL:
      KernelCvarSignal(&(curr_proc->uc));
      break;

    case YALNIX_CVAR_BROADCAST:
      KernelCvarBroadcast(&(curr_proc->uc));
      break;

    case YALNIX_CVAR_WAIT:
      KernelCvarWait(&(curr_proc->uc));
      break;

    case YALNIX_RECLAIM:
      KernelReclaim(&(curr_proc->uc));
      break;

    case YALNIX_SEM_UP:
      KernelSemUp(&(curr_proc->uc));
      break;

    case YALNIX_SEM_DOWN:
      KernelSemDown(&(curr_proc->uc));
      break;

    case YALNIX_SEM_INIT:
      KernelSemInit(&(curr_proc->uc));
      break;

    default:
      TracePrintf(0, "Syscall not implemented\n");
      TracePrintf(0, "Address %p\n", uc->addr);
      break;
  }

  curr_proc = get_running_proc();

  memcpy(uc, &(curr_proc->uc), sizeof(UserContext));
}

/* Handle the clock interrupt, update ticks, and reschedule processes. */
static void trap_clock_handler(UserContext *uc) {
  increment_ticks();
  wake_waiters();
  wake_sleepers();
  
  TracePrintf(2, "A trap clock occurred. \n");


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

/* Default handler for any trap code that has not been implemented. */
static void trap_not_implemented(UserContext *uc) {
  TracePrintf(0, "A trap that isn't implemented occurred\n");
  TracePrintf(0, "Addr: %#x, Maperr? %d, Accerr? %d\n", uc->addr, uc->code == YALNIX_MAPERR, uc->code == YALNIX_ACCERR);
  Halt();
}

/* Terminate the process on a floating point or math exception. */
static void trap_math_handler(UserContext *unused) {
  KernelExit(NULL);
}

/* Terminate the process on an illegal instruction trap. */
static void trap_illegal_handler(UserContext *unused) {
  KernelExit(NULL);
}

static void trap_memory_handler(UserContext *uc) {
  if (!expand_stack((int)(long) uc->addr)) KernelExit(NULL);
}
