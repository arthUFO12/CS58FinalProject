#include "interrupt.h"
#include "hardware.h"

static void trap_kernel_handler(UserContext *uc) {
  TracePrintf(1, "A trap kernel with code %x\n", uc->code);
}

static void trap_clock_handler(UserContext *uc) {
  TracePrintf(1, "A trap clock occurred\n");
}

static void trap_not_implemented(UserContext *uc) {
  TracePrintf(1, "A trap that isn't implemented occurred");
}

static void *interrupt_vector[TRAP_VECTOR_SIZE];

void init_interrupt_vector() {
  for (int i = 0; i < TRAP_VECTOR_SIZE; i++) {
    interrupt_vector[i] = (void*) trap_not_implemented;
  }
  interrupt_vector[TRAP_KERNEL] = (void*) trap_kernel_handler;
  interrupt_vector[TRAP_CLOCK] = (void*) trap_clock_handler;

  WriteRegister(REG_VECTOR_BASE, (unsigned int) (long) interrupt_vector);
}
