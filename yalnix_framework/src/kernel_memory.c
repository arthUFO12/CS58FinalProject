/**
 * @file kernel_memory.c
 * @brief Sets up and defines functions that control virtual memory for kernel
 */

#include "kernel_memory.h"
#include "frame_tracking.h"
#include "hardware.h"
#include "ykernel.h"
#include "ylib.h"

static bool vm_enabled = false;

/** Page table holding kernel address space */
static pte_t region0_pt[REGION0_VPNS];

static int current_brk_page;

static bool alloc_page(int vpn, int prot);
static bool dealloc_page(int vpn);
static int unmap_no_free(int vpn);

static void undo_allocation(int first_vpn, int last_vpn);
static bool free_no_unmap(int vpn);

void enable_vm(void) {
  WriteRegister(REG_VM_ENABLE, 1);
  vm_enabled = true;
}

void init_kernel_brk() { current_brk_page = _orig_kernel_brk_page; }

bool init_region0_pt(pcb_t *idle_pcb) {
  TracePrintf(0, "Starting region0 pt initialization\n");

  memset(region0_pt, 0x00, sizeof(region0_pt));

  for (int vpn = _first_kernel_text_page; vpn < _first_kernel_data_page;
       vpn++) {
    TracePrintf(0, "Acquiring vpn number 0x%x for kernel text\n", vpn);
    if (!acquire_frame(vpn))
      return false;
    create_pte(region0_pt, vpn, vpn, PROT_READ | PROT_EXEC);
  }

  for (int vpn = _first_kernel_data_page; vpn < current_brk_page; vpn++) {
    TracePrintf(0, "Acquiring vpn number 0x%x for kernel data\n", vpn);

    if (!acquire_frame(vpn))
      return false;
    create_pte(region0_pt, vpn, vpn, PROT_READ | PROT_WRITE);
  }

  for (int vpn = K_STACK_BASE_VPN; vpn < K_STACK_LIMIT_VPN; vpn++) {
    TracePrintf(0, "Acquiring vpn number 0x%x for kernel stack\n", vpn);

    if (!acquire_frame(vpn))
      return false;
    create_pte(region0_pt, vpn, vpn, PROT_READ | PROT_WRITE);
  }

  memcpy(idle_pcb->ks_pt, region0_pt + K_STACK_BASE_VPN,
         K_STACK_VPNS * sizeof(pte_t));

  TracePrintf(0, "Writing region 0 page table to register\n");

  WriteRegister(REG_PTBR0, (unsigned int)(long)region0_pt);
  WriteRegister(REG_PTLR0, REGION0_VPNS);

  return true;
}

int SetKernelBrk(void *addr) {
  int heap_page = UP_TO_PAGE(addr) >> PAGESHIFT;
  int stack_page = DOWN_TO_PAGE(KERNEL_STACK_BASE) >> PAGESHIFT;

  TracePrintf(
      0, "Setting Brk with,\n heap_page=%d\n stack_page=%d\n original_brk=%d\n",
      heap_page, stack_page, _orig_kernel_brk_page);
  if (heap_page < _orig_kernel_brk_page || heap_page >= stack_page - 2)
    return ERROR;

  if (!vm_enabled) {
    current_brk_page = heap_page;
    return 0;
  }

  if (heap_page > current_brk_page) {
    for (int vpn = current_brk_page; vpn < heap_page; vpn++) {
      if (!alloc_page(vpn, PROT_READ | PROT_WRITE)) {
        undo_allocation(current_brk_page, vpn);
        return ERROR;
      }
    }
  } else {
    undo_allocation(heap_page, current_brk_page);
  }

  current_brk_page = heap_page;

  return 0;
}

KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *unused) {
  pcb_t *new_pcb = (pcb_t *)new_pcb_p;
  int true_brk_page = current_brk_page;

  if (SetKernelBrk((void *)(long)((true_brk_page + K_STACK_VPNS)
                                  << PAGESHIFT)) == ERROR) {
    return NULL;
  }

  memcpy((void *)(long)(true_brk_page << PAGESHIFT), (void *)KERNEL_STACK_BASE, KERNEL_STACK_MAXSIZE);

  pte_t *ks_pt = new_pcb->ks_pt;

  for (int vpn = true_brk_page; vpn < true_brk_page + K_STACK_VPNS; vpn++) {
    int pfn = unmap_no_free(vpn);
    int shift = vpn - true_brk_page;

    ks_pt[shift].valid = 1;
    ks_pt[shift].prot = PROT_READ | PROT_WRITE;
    ks_pt[shift].pfn = pfn;
  }

  TracePrintf(0,
              "Set brk page to %d temporarily to copy kernel stack. True brk "
              "page is %d\n",
              current_brk_page, true_brk_page);
  current_brk_page = true_brk_page;

  memcpy(&(new_pcb->kc), kc_in, sizeof(KernelContext));

  return kc_in;
}

KernelContext *KCSwitch(KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p) {

  pcb_t *curr_pcb = (pcb_t *)curr_pcb_p;
  pcb_t *next_pcb = (pcb_t *)next_pcb_p;

  if (curr_pcb != NULL)
    memcpy(&(curr_pcb->kc), kc_in, sizeof(KernelContext));

  memcpy(region0_pt + K_STACK_BASE_VPN, next_pcb->ks_pt,
         K_STACK_VPNS * sizeof(pte_t));

  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_KSTACK);

  return &(next_pcb->kc);
}

static bool alloc_page(int vpn, int prot) {
  int pfn = find_frame();

  if (pfn == ERROR)
    return false;

  if (vpn < REGION0_VPNS && vpn >= 0) {
    create_pte(region0_pt, vpn, pfn, prot);
    return true;
  }

  return false;
}

void deallocate_kernel_stack() {
  for (int vpn = K_STACK_BASE_VPN; vpn < K_STACK_LIMIT_VPN; vpn++) {
    free_no_unmap(vpn);
  }
}

static bool dealloc_page(int vpn) {

  if (vpn >= REGION0_VPNS || vpn < 0)
    return false;

  if (region0_pt[vpn].valid == 0) {
    return false;
  }

  int pfn = destroy_pte(region0_pt, vpn);
  WriteRegister(REG_TLB_FLUSH, vpn << PAGESHIFT);

  if (!free_frame(pfn)) {
    return false;
  }

  return true;
}

static bool free_no_unmap(int vpn) {
  if (vpn >= REGION0_VPNS || vpn < 0)
    return false;

  if (region0_pt[vpn].valid == 0)
    return false;

  if (!free_frame(region0_pt[vpn].pfn))
    return false;

  return true;
}

/**
 * @brief Deallocate page but do not return to frame buffer
 *
 * @param vpn Virtual Page Number
 * @return Page Frame Number or ERROR
 */
static int unmap_no_free(int vpn) {
  if (vpn < REGION0_VPNS && vpn >= 0) {
    int pfn = destroy_pte(region0_pt, vpn);
    WriteRegister(REG_TLB_FLUSH, vpn << PAGESHIFT);
    return pfn;
  }
  return ERROR;
}

static void undo_allocation(int first_vpn, int last_vpn) {
  for (int vpn = first_vpn; vpn < last_vpn; vpn++)
    dealloc_page(vpn);
}
