#include "user_memory.h"
#include "frame_tracking.h"
#include "pcb.h"
#include "ylib.h"

/*
 * Region 1 memory management for user processes.
 * This module allocates and frees user pages, manages brk growth,
 * and switches the active region 1 page table for context switches.
 */

static mem_ctx_t *mem_ctx;

static bool alloc_page(int vpn, int prot);
static bool dealloc_page(int vpn);
static int unmap_no_free(int vpn);
static void change_prot(int vpn, int prot);
static void undo_allocation(int first_vpn, int last_vpn);
static void destroy_pt(pte_t *pt, int first, int last);

/*
 * Initialize the region 1 page table registers for the idle process.
 * The region 1 page table base is written into the hardware registers.
 */
bool init_region1_pt(pcb_t *idle) {
  mem_ctx = &(idle->mem_ctx);

  TracePrintf(2, "Starting region 1 pt initialization\n");
  TracePrintf(2, "Writing region1 page table to register\n");

  WriteRegister(REG_PTBR1, (unsigned int)(long)mem_ctx->region1_pt);
  WriteRegister(REG_PTLR1, REGION1_VPNS);

  mem_ctx->txt_start_page = REGION1_BASE_VPN;
  mem_ctx->data_start_page = REGION1_BASE_VPN;
  mem_ctx->orig_brk_page = REGION1_BASE_VPN;
  mem_ctx->curr_brk_page = REGION1_BASE_VPN;

  return alloc_page(DOWN_TO_PAGE(VMEM_1_LIMIT - 1) >> PAGESHIFT,
                    PROT_READ | PROT_WRITE);
}

/*
 * KernelBrk_Impl adjusts the user heap break for the current process.
 * It allocates or deallocates region 1 pages as required by the new break.
 */
int KernelBrk_Impl(void *addr, void *sp) {
  int heap_page = UP_TO_PAGE(addr) >> PAGESHIFT;
  int stack_page = DOWN_TO_PAGE(sp) >> PAGESHIFT;
  int curr_brk_page = mem_ctx->curr_brk_page;

  TracePrintf(3,
              "Setting User Brk with,\n heap_page=%d\n stack_page=%d\n "
              "original_brk=%d\n",
              heap_page, stack_page, curr_brk_page);

  if (heap_page < mem_ctx->orig_brk_page || heap_page >= stack_page - 1)
    return ERROR;

  if (heap_page > curr_brk_page) {
    for (int vpn = curr_brk_page; vpn < heap_page; vpn++) {
      if (!alloc_page(vpn, PROT_READ | PROT_WRITE)) {
        undo_allocation(curr_brk_page, vpn);
        return ERROR;
      }
    }
  } else {
    undo_allocation(heap_page, curr_brk_page);
  }

  mem_ctx->curr_brk_page = heap_page;

  return 0;
}

bool expand_stack(int sp) {
  if (sp < VMEM_1_BASE || sp >= VMEM_1_LIMIT) return false;

  int stack_page = DOWN_TO_PAGE(sp) >> PAGESHIFT;
  if (stack_page < mem_ctx->curr_brk_page + 1) return false;

  int vpn = stack_page;
  while (vpn < REGION1_VPNS + REGION0_VPNS && !mem_ctx->region1_pt[vpn - REGION1_BASE_VPN].valid) {
    alloc_page(vpn, PROT_READ | PROT_WRITE);
    vpn++;
  }
  
  return true;
}
/*
 * Copy the current process's user memory layout to a new PCB for fork.
 * The caller provides the user context to duplicate and the new child PCB.
 */
bool UCCopy(UserContext *uc_in, pcb_t *new_pcb) {
  int txt_page = mem_ctx->txt_start_page;
  int data_page = mem_ctx->data_start_page;
  int brk_page = mem_ctx->curr_brk_page;
  int stack_page = DOWN_TO_PAGE(uc_in->sp) >> PAGESHIFT;
  void *brk_addr = (void *)(brk_page << PAGESHIFT);

  if (brk_page >= stack_page) {
    return false;
  }

  for (int vpn = txt_page; vpn < data_page; vpn++) {
    if (!alloc_page(brk_page, PROT_READ | PROT_WRITE)) {
      destroy_pt(new_pcb->mem_ctx.region1_pt, 0, vpn - REGION1_BASE_VPN);
      return false;
    }
    memcpy(brk_addr, (void *)(long)(vpn << PAGESHIFT), PAGESIZE);

    int pfn = unmap_no_free(brk_page);
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].pfn = pfn;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].valid = 1;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].prot =
        PROT_READ | PROT_EXEC;
  }

  for (int vpn = data_page; vpn < brk_page; vpn++) {
    if (!alloc_page(brk_page, PROT_READ | PROT_WRITE)) {
      destroy_pt(new_pcb->mem_ctx.region1_pt, 0, vpn - REGION1_BASE_VPN);
      return false;
    }
    memcpy(brk_addr, (void *)(vpn << PAGESHIFT), PAGESIZE);

    int pfn = unmap_no_free(brk_page);
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].pfn = pfn;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].valid = 1;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].prot =
        PROT_READ | PROT_WRITE;
  }

  for (int vpn = stack_page; vpn < REGION1_LIMIT_VPN; vpn++) {
    if (!alloc_page(brk_page, PROT_READ | PROT_WRITE)) {
      destroy_pt(new_pcb->mem_ctx.region1_pt, 0, vpn - REGION1_BASE_VPN);
      return false;
    }
    memcpy(brk_addr, (void *)(long)(vpn << PAGESHIFT), PAGESIZE);

    int pfn = unmap_no_free(brk_page);
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].pfn = pfn;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].valid = 1;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].prot =
        PROT_READ | PROT_WRITE;
  }

  new_pcb->mem_ctx.txt_start_page = txt_page;
  new_pcb->mem_ctx.data_start_page = data_page;
  new_pcb->mem_ctx.curr_brk_page = brk_page;
  new_pcb->mem_ctx.orig_brk_page = mem_ctx->orig_brk_page;

  memcpy(&(new_pcb->uc), uc_in, sizeof(UserContext));
  return true;
}

/* Allocate a contiguous range of region 1 pages with the requested protection. */
bool alloc_region(int start_vpn, int end_vpn, int prot) {
  if (start_vpn < REGION1_BASE_VPN ||
      end_vpn > REGION1_BASE_VPN + REGION1_VPNS) {
    return false;
  }

  for (int vpn = start_vpn; vpn < end_vpn; vpn++) {
    if (!alloc_page(vpn, prot)) {
      undo_allocation(start_vpn, vpn);
      return false;
    }
  }

  return true;
}

/* Change protection bits for a range of region 1 pages. */
void prot_region(int start_vpn, int end_vpn, int prot) {
  if (start_vpn < REGION1_BASE_VPN ||
      end_vpn > REGION1_BASE_VPN + REGION1_VPNS) {
    return;
  }

  for (int vpn = start_vpn; vpn < end_vpn; vpn++) {
    change_prot(vpn, prot);
  }
}

/*
 * Switch the active region 1 page table to the specified next process.
 */
void UCSwitch(pcb_t *next_pcb) {
  mem_ctx = &(next_pcb->mem_ctx);

  WriteRegister(REG_PTBR1, (unsigned int)(long)mem_ctx->region1_pt);
  WriteRegister(REG_PTLR1, REGION1_VPNS);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
}

/* Deallocate and unmap all region 1 pages for the current process. */
void deallocate_region1(void) {
  undo_allocation(REGION1_BASE_VPN, REGION1_LIMIT_VPN);
}

static bool alloc_page(int vpn, int prot) {
  
  vpn = vpn - REGION1_BASE_VPN;
  int pfn = find_frame();
  if (pfn == -1)
    return false;

  if (vpn < REGION1_VPNS && vpn >= 0) {
    create_pte(mem_ctx->region1_pt, vpn, pfn, prot);
    return true;
  }

  return false;
}

static bool dealloc_page(int vpn) {
  int relative_vpn = vpn - REGION1_BASE_VPN;
  if (relative_vpn < REGION1_VPNS && relative_vpn >= 0) {
    if (mem_ctx->region1_pt[relative_vpn].valid == 0) {
      return false;
    }

    int pfn = destroy_pte(mem_ctx->region1_pt, relative_vpn);
    WriteRegister(REG_TLB_FLUSH, vpn << PAGESHIFT);
    if (!free_frame(pfn))
      return false;

    return true;
  }

  return false;
}

static void change_prot(int vpn, int prot) {
  int relative_vpn = vpn - REGION1_BASE_VPN;
  if (relative_vpn >= REGION1_VPNS || relative_vpn < 0) {
    return;
  }
  if (mem_ctx->region1_pt[relative_vpn].valid == 0) {
    return;
  }

  mem_ctx->region1_pt[relative_vpn].prot = prot;
  WriteRegister(REG_TLB_FLUSH, vpn << PAGESHIFT);
}

static int unmap_no_free(int vpn) {
  int relative_vpn = vpn - REGION1_BASE_VPN;
  if (relative_vpn < REGION1_VPNS && relative_vpn >= 0) {
    int pfn = destroy_pte(mem_ctx->region1_pt, relative_vpn);
    WriteRegister(REG_TLB_FLUSH, vpn << PAGESHIFT);
    return pfn;
  }

  return -1;
}

static void undo_allocation(int first_vpn, int last_vpn) {
  for (int vpn = first_vpn; vpn < last_vpn; vpn++)
    dealloc_page(vpn);
}

/*
 * Free frame resources for a copied page table without unmapping them.
 * Used to clean up page tables when a copy fails.
 */
static void destroy_pt(pte_t *pt, int first, int last) {
  for (int vpn = first; vpn < last; vpn++) {
    if (pt[vpn].valid == 1) {
      free_frame(pt[vpn].pfn);
    }
  }
}
