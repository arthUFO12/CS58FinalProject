#include "interrupt.h"
#include "hardware.h"

void trap_kernel_handler(UserContext *uc) {
  TracePrintf(1, "A trap kernel with code %x\n", uc->code);
}

void trap_clock_handler(UserContext *uc) {
  TracePrintf(1, "A trap clock occurred\n");
}

void trap_not_implemented(UserContext *uc) {
  TracePrintf(1, "A trap that isn't implemented occurred");
}

void *interrupt_vector[TRAP_VECTOR_SIZE];

void init_interrupt_vector() {
  for (int i = 0; i < TRAP_VECTOR_SIZE; i++) {
    interrupt_vector[i] = trap_not_implemented;
  }
  interrupt_vector[TRAP_KERNEL] = trap_kernel_handler;
  interrupt_vector[TRAP_CLOCK] = trap_clock_handler;
}
