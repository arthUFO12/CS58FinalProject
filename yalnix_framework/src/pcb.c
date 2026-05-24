
#include "pcb.h"
#include "ykernel.h"
#include "ylib.h"

/*
 * Process control block helpers.
 * This module creates and destroys PCB structures used by the scheduler
 * and memory management components.
 */

/*
 * Allocate and initialize a new process control block.
 * - num_ks_pages: number of kernel stack page table entries.
 * - num_region1_pte: number of region 1 page table entries.
 * - uc: initial user context for the new process.
 */
pcb_t* create_new_pcb(int num_ks_pages, int num_region1_pte, UserContext* uc) {
  pcb_t* pcb = calloc(1, sizeof(pcb_t));

  if (pcb == NULL) {
    return NULL;
  }

  pcb->ks_pt = calloc(1, num_ks_pages * sizeof(pte_t));
  if (pcb->ks_pt == NULL) {
    free(pcb);
    return NULL;
  }

  pcb->mem_ctx.region1_pt = calloc(1, num_region1_pte * sizeof(pte_t));
  if (pcb->mem_ctx.region1_pt == NULL) {
    free(pcb->ks_pt);
    free(pcb);
    return NULL;
  }

  pcb->pid = helper_new_pid(pcb->mem_ctx.region1_pt);
  pcb->state = READY;

  memcpy(&(pcb->uc), uc, sizeof(UserContext));

  return pcb;
}

/* Release all PCB resources and retire the process ID. */
void clean_up_pcb(pcb_t* pcb) {
  helper_retire_pid(pcb->pid);
  free(pcb->ks_pt);
  free(pcb->mem_ctx.region1_pt);
  free(pcb);
}
