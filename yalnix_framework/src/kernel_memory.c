
#include "kernel_memory.h"
#include "pcb.h"
#include "frame_tracking.h"
#include "hardware.h"
#include "ykernel.h"
#include "ylib.h"

static bool vm_enabled = false;

static pte_t *region0_pt;
static pte_t *region1_pt;

static int current_brk_page;

static void create_pte(pte_t *base, int vpn, int pfn, int prot);
static int destroy_pte(pte_t *base, int vpn);
static bool alloc_page(int vpn, int prot);
static bool dealloc_page(int vpn);
static int unmap_no_free(int vpn);

static void undo_allocation(int first_vpn, int last_vpn);

void enable_vm() {
  WriteRegister(REG_VM_ENABLE, 1);
  vm_enabled = true;
}

void init_brk() {
  current_brk_page = _orig_kernel_brk_page;
}


pte_t* get_region1_pt(void) {
  return region1_pt;
}


bool setup_region0_pt(void) {
  region0_pt = calloc(REGION0_VPNS, sizeof(pte_t));
  
  TracePrintf(0, "Starting region0 pt initialization\n");

  if (region0_pt == NULL)
    return false;

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

  TracePrintf(0, "Writing region 0 page table to register\n");

  WriteRegister(REG_PTBR0, (unsigned int)(long) region0_pt);
  WriteRegister(REG_PTLR0, REGION0_VPNS);

  return true;
}

bool setup_region1_pt(void) {
  region1_pt = calloc(REGION1_VPNS, sizeof(pte_t));
  TracePrintf(0, "Starting region 1 pt initialization\n");

  if (region1_pt == NULL) {
    return false;
  }

  TracePrintf(0, "Writing region1 page table to register\n");

  WriteRegister(REG_PTBR1, (unsigned int)(long)region1_pt);
  WriteRegister(REG_PTLR1, REGION1_VPNS);

  return alloc_page(DOWN_TO_PAGE(VMEM_1_LIMIT - 1) >> PAGESHIFT, PROT_READ | PROT_WRITE);
}

int SetKernelBrk(void *addr) {
  int heap_page = UP_TO_PAGE(addr) >> PAGESHIFT;
  int stack_page = DOWN_TO_PAGE(KERNEL_STACK_BASE) >> PAGESHIFT;
  
  TracePrintf(0, "Setting Brk with,\n heap_page=%d\n stack_page=%d\n original_brk=%d\n", heap_page, stack_page, _orig_kernel_brk_page);
  if (heap_page < _orig_kernel_brk_page || heap_page >= stack_page - 1)
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

static void undo_allocation(int first_vpn, int last_vpn) {
  for (int vpn = first_vpn; vpn < last_vpn; vpn++)
    dealloc_page(vpn);
}

KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *unused) {
  pcb_t *new_pcb = (pcb_t *)new_pcb_p;
  int true_brk_page = current_brk_page;

  if (SetKernelBrk((void *)(long)((true_brk_page + NUM_K_STACK_VPNS)
                                  << PAGESHIFT)) == -1) {
    return NULL;
  }

  memcpy((void *)(long)(true_brk_page << PAGESHIFT), (void *)KERNEL_STACK_BASE,
         KERNEL_STACK_MAXSIZE);

  pte_t *ks_pt = new_pcb->ks_pt;

  for (int vpn = true_brk_page; vpn < true_brk_page + NUM_K_STACK_VPNS; vpn++) {
    int pfn = unmap_no_free(vpn);
    int shift = vpn - true_brk_page;

    ks_pt[shift].valid = 1;
    ks_pt[shift].prot = PROT_READ | PROT_WRITE;
    ks_pt[shift].pfn = pfn;
  }

  current_brk_page = true_brk_page;

  memcpy(&(new_pcb->kc), kc_in, sizeof(KernelContext));

  return kc_in;
}

KernelContext *KCSwitch(KernelContext *kc_in, void *curr_pcb_p,
                        void *next_pcb_p) {

  pcb_t *curr_pcb = (pcb_t *)curr_pcb_p;
  pcb_t *next_pcb = (pcb_t *)next_pcb_p;

  memcpy(&(curr_pcb->kc), kc_in, sizeof(KernelContext));

  memcpy(region0_pt + K_STACK_BASE_VPN, next_pcb->ks_pt,
         NUM_K_STACK_VPNS * sizeof(pte_t));

  WriteRegister(REG_PTBR1, (unsigned long)next_pcb->region1_pt);
  WriteRegister(REG_PTLR1, REGION1_VPNS);

  region1_pt = next_pcb->region1_pt;

  return kc_in;
}

static bool alloc_page(int vpn, int prot) {
  int pfn = find_frame();

  if (pfn == -1)
    return false;

  if (vpn < REGION0_VPNS) {
    create_pte(region0_pt, vpn, pfn, prot);
  } else {
    vpn = vpn - REGION0_VPNS;
    create_pte(region1_pt, vpn, pfn, prot);
  }

  return true;
}

static bool dealloc_page(int vpn) {

  if (vpn < REGION0_VPNS) {
    int pfn = destroy_pte(region0_pt, vpn);
    if (!free_frame(pfn))
      return false;
  } else {
    vpn = vpn - REGION0_VPNS;
    int pfn = destroy_pte(region1_pt, vpn);
    if (!free_frame(pfn))
      return false;
  }

  return true;
}

static int unmap_no_free(int vpn) {

  if (vpn < REGION0_VPNS)
    return destroy_pte(region0_pt, vpn);
  else {
    vpn = vpn - REGION0_VPNS;
    return destroy_pte(region1_pt, vpn);
  }
}

static int destroy_pte(pte_t *base, int vpn) {
  base[vpn].valid = 0;
  return base[vpn].pfn;
}

static void create_pte(pte_t *base, int vpn, int pfn, int prot) {
  base[vpn].valid = 1;
  base[vpn].pfn = pfn;
  base[vpn].prot = prot;
}
