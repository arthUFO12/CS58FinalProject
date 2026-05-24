#include "pcb.h"
#include "ykernel.h"
#include "ylib.h"

#define CHILD_ARR_SIZE 10

/*
 * Process control block management.
 * This module creates PCBs, tracks parent-child relationships, and
 * cleans up process resources when a process exits.
 */

/* Allocate and initialize a new PCB with kernel stack and user page tables. */
pcb_t *create_new_pcb(int num_ks_pages, int num_region1_pte, UserContext *uc) {
  pcb_t *pcb = calloc(1, sizeof(pcb_t));
  if (pcb == NULL)
    goto cleanup_pcb;

  pcb->ks_pt = calloc(1, num_ks_pages * sizeof(pte_t));
  if (pcb->ks_pt == NULL)
    goto cleanup_ks_pt;

  pcb->mem_ctx.region1_pt = calloc(1, num_region1_pte * sizeof(pte_t));
  if (pcb->mem_ctx.region1_pt == NULL)
    goto cleanup_region1_pt;

  pcb->relations.children = calloc(CHILD_ARR_SIZE, sizeof(pcb_t *));
  if (pcb->relations.children == NULL)
    goto cleanup_children;

  pcb->pid = helper_new_pid(pcb->mem_ctx.region1_pt);
  pcb->state = READY;
  pcb->relations.has_parent = true;
  pcb->relations.arr_size = CHILD_ARR_SIZE;
  memcpy(&(pcb->uc), uc, sizeof(UserContext));

  return pcb;

cleanup_children:
  free(pcb->mem_ctx.region1_pt);
cleanup_region1_pt:
  free(pcb->ks_pt);
cleanup_ks_pt:
  free(pcb);
cleanup_pcb:
  return NULL;
}

/*
 * Search for an exited child process, return its exit code, and compact the
 * parent's child list.
 */
bool find_exited_child(pcb_t *parent, int *output) {
  pcb_t **children = parent->relations.children;
  int num_children = parent->relations.num_children;
  int arr_size = parent->relations.arr_size;
  int i = 0;

  for (; i < num_children; i++) {
    if (children[i]->state == EXITED) {
      if (output != NULL)
        *output = children[i]->exit_code;
      free(children[i]);
      goto compact;
    }
  }

  return false;

compact:
  for (; i + 1 < num_children; i++)
    children[i] = children[i + 1];

  children[num_children - 1] = NULL;
  parent->relations.num_children--;

  if (num_children < arr_size / 2) {
    arr_size /= 2;
    pcb_t **temp = calloc(arr_size, sizeof(pcb_t *));
    if (temp == NULL)
      goto end;

    memcpy(temp, children, num_children * sizeof(pcb_t *));
    free(children);
    parent->relations.arr_size = arr_size;
    parent->relations.children = children = temp;
  }

end:
  return true;
}

/* Add a child process to the parent's child list, resizing if necessary. */
bool add_child_proc(pcb_t *parent, pcb_t *child) {
  pcb_t **children = parent->relations.children;
  int num_children = parent->relations.num_children;
  int arr_size = parent->relations.arr_size;

  if (num_children == arr_size) {
    arr_size *= 2;
    pcb_t **temp = calloc(arr_size, sizeof(pcb_t *));
    if (temp == NULL)
      return false;

    memcpy(temp, children, num_children * sizeof(pcb_t *));
    free(children);
    parent->relations.arr_size = arr_size;
    parent->relations.children = children = temp;
  }

  children[num_children] = child;
  parent->relations.num_children++;
  return true;
}

/* Free PCB resources and mark children as orphaned if necessary. */
void retire_pcb(pcb_t *pcb) {
  pcb_t **children = pcb->relations.children;
  int num_children = pcb->relations.num_children;

  for (int i = 0; i < num_children; i++) {
    if (children[i]->state == EXITED)
      free(children[i]);
    else
      children[i]->relations.has_parent = false;
  }

  helper_retire_pid(pcb->pid);
  free(pcb->ks_pt);
  free(pcb->mem_ctx.region1_pt);
  free(pcb->relations.children);

  if (!pcb->relations.has_parent)
    free(pcb);
}
