/**
 * @file kernel_memory.c
 * @brief Sets up and defines functions that control virtual memory for kernel
 */

#include "kernel_memory.h"
#include "hardware.h"
#include "ykernel.h"
#include "ylib.h"

#include "frame_tracking.h"

static bool vm_enabled = false;

/** Page table holding kernel address space */
static pte_t region0_pt[REGION0_VPNS];

static int current_brk_page;

/* Helper Definitions */
static bool alloc_page(int vpn, int prot);
static bool dealloc_page(int vpn);
static int unmap_no_free(int vpn);
static void undo_allocation(int first_vpn, int last_vpn);

void enable_vm(void) {
  WriteRegister(REG_VM_ENABLE, 1);
  vm_enabled = true;
}

void init_kernel_brk(void) {
  /*
   set where kernel heap starts
   */
}

bool init_region0_pt(pcb_t *idle_pcb) {
  /*
   clear memory for table

   map kernel text (readonly + exec)
   map kernel data (read-write)
   map kernel stack (read-write)

   copy stack into idle PCB

   add table to hardware register

  return status
   */
}

int SetKernelBrk(void *addr) {
  /*
   convert addr to page index

   check if heap would intersect stack

   allocate or free pages

   update break pointer

   return status
   */
}

KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *unused) {
  /*
  extend kernel break to new page

  copy current kernel stack into new page

  give new page to new_pcb

  restore original kernel break

  copy kernel context to new_pcb

  return old context
  */
}

KernelContext *KCSwitch(KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p) {

  /*
  save current kernel registers into pcb

  replace stack with new_pcb stack

  flush tlb

  return next_pcb kernel ctxt
  */
}
