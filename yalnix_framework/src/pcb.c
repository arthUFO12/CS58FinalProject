#include "pcb.h"
#include "ykernel.h"
#include "ylib.h"

/**
 * @brief create a new process control block
 *
 * @param num_ks_pages  Number of pages
 * @param num_region1_pte Page table
 * @param uc  Usercontext
 * @return Pointer to new process control block
 */
pcb_t *create_new_pcb(int num_ks_pages, int num_region1_pte, UserContext *uc) {
  /*
  // allocate memory for new pcb
  pcb_t *pcb = calloc(1, sizeof(pcb_t));
  pcb->ks_pt = calloc(1, num_ks_pages * sizeof(pte_t));
  pcb->mem_ctx.region1_pt = calloc(1, num_region1_pte * sizeof(pte_t));

  // get new process id for pcb
  pcb->pid = helper_new_pid(pcb->mem_ctx.region1_pt);

  // indicate process ready
  pcb->state = READY;

  // copy over usercontext
  memcpy(&(pcb->uc), uc, sizeof(UserContext));

  return pcb;
  */
}

void clean_up_pcb(pcb_t *pcb) {
  /*
  helper_retire_pid(pcb->pid);
  free(pcb->ks_pt);
  free(pcb->mem_ctx.region1_pt);
  free(pcb);
  */
}
