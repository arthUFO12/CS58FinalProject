
#include "pcb.h"
#include "ykernel.h"
#include "ylib.h"

pcb_t *create_new_pcb(int num_ks_pages, int num_region1_pte, UserContext *uc) {
  pcb_t *pcb = calloc(1, sizeof(pcb_t));

  if (pcb == NULL) {
    return NULL;
  }

  pcb->ks_pt = calloc(1, num_ks_pages * sizeof(pte_t));
  if (pcb->ks_pt == NULL) {
    return NULL;
  }

  pcb->mem_ctx.region1_pt = calloc(1, num_region1_pte * sizeof(pte_t));
  if (pcb->mem_ctx.region1_pt == NULL)
    return NULL;

  pcb->pid = helper_new_pid(pcb->mem_ctx.region1_pt);
  pcb->state = READY;

  memcpy(&(pcb->uc), uc, sizeof(UserContext));

  return pcb;
}

void clean_up_pcb(pcb_t *pcb) {
  helper_retire_pid(pcb->pid);
  free(pcb->ks_pt);
  free(pcb->mem_ctx.region1_pt);
  free(pcb);
}
