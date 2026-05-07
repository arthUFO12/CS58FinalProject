
#include "frame_tracking.h"
#include "hardware.h"
#include "interrupt.h"
#include "kernel_memory.h"
#include "pcb.h"
#include <assert.h>

void do_idle(void);

void do_idle(void) {
  while (1) {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}

void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt) {
  initialize_frame_tracking(pmem_size);
  assert(setup_region0_pt());
  assert(setup_region1_pt());

  init_interrupt_vector();

  enable_vm();

  init_pcb_queue();
  set_up_uc(uctxt, do_idle, VMEM_1_LIMIT);
  pcb_t *idle_pcb = create_init_pcb(NUM_K_STACK_VPNS, REGION0_VPNS, uctxt);

  assert(idle_pcb);
}
