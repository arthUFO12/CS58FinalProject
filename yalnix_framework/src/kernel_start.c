
#include "frame_tracking.h"
#include "hardware.h"
#include "interrupt.h"
#include "kernel_memory.h"
#include "pcb.h"
#include <assert.h>

static void do_idle(void);

static void do_idle(void) {
  while (1) {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}

void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt) {
  TracePrintf(0, "Pmem_size %u\n", pmem_size);

  initialize_frame_tracking(pmem_size);
  init_brk();

  TracePrintf(0, "Initialized frame tracking\n");

  assert(setup_region0_pt());
  assert(setup_region1_pt());

  TracePrintf(0, "Starting interrupt vector initialization\n");
  init_interrupt_vector();

  enable_vm();

  init_pcb_queue();
  set_up_uc(uctxt, do_idle, VMEM_1_LIMIT - 1);
  pcb_t *idle_pcb = create_idle_pcb(NUM_K_STACK_VPNS, get_region1_pt(), uctxt);
  pcb_t *init_pcb = create_new_pcb(NUM_K_STACK_VPNS, REGION1_VPNS, uctxt);
  assert(idle_pcb);

  KCCopy(&(idle_pcb->kc), init_pcb, NULL);

  enque_pcb(MAIN_QUEUE, init_pcb);
}
