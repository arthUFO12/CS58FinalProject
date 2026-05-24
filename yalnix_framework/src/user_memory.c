#include "user_memory.h"
#include "frame_tracking.h"
#include "pcb.h"
#include "ylib.h"

#define REGION0_VPNS (VMEM_0_SIZE >> PAGESHIFT)
#define REGION1_VPNS (VMEM_1_SIZE >> PAGESHIFT)
#define REGION1_BASE_VPN REGION0_VPNS
#define REGION1_LIMIT_VPN (REGION1_BASE_VPN + REGION1_VPNS)

/*
 * User memory management for Region 1.
 * Handles process-specific page table setup, brk handling, and copy-on-load logic.
 */

static mem_ctx_t *mem_ctx;

static bool alloc_page(int vpn, int prot);
static bool dealloc_page(int vpn);
static int unmap_no_free(int vpn);
static void change_prot(int vpn, int prot);

/* Initialize the Region 1 page table for the idle process. */
bool init_region1_pt(pcb_t* idle) {
  mem_ctx = &(idle->mem_ctx);
  TracePrintf(0, "Starting region 1 pt initialization\n");
  TracePrintf(0, "Writing region1 page table to register\n");

  WriteRegister(REG_PTBR1, (unsigned int)(long) mem_ctx->region1_pt);
  WriteRegister(REG_PTLR1, REGION1_VPNS);

  mem_ctx->txt_start_page = REGION1_BASE_VPN;
  mem_ctx->data_start_page = REGION1_BASE_VPN;
  mem_ctx->orig_brk_page = REGION1_BASE_VPN;
  mem_ctx->curr_brk_page = REGION1_BASE_VPN;

  return alloc_page(DOWN_TO_PAGE(VMEM_1_LIMIT - 1) >> PAGESHIFT, PROT_READ | PROT_WRITE);
}

/* Adjust the user heap break pointer and allocate or free pages as needed. */
int KernelBrk_Impl(void* addr, void* sp) {
  int heap_page = UP_TO_PAGE(addr) >> PAGESHIFT;
  int stack_page = DOWN_TO_PAGE(sp) >> PAGESHIFT;
  int curr_brk_page = mem_ctx->curr_brk_page;

  TracePrintf(0, "Setting User Brk with,\n heap_page=%d\n stack_page=%d\n old_brk=%d\n", heap_page, stack_page, curr_brk_page);

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

/* Copy user memory context and page mappings for a new process. */
UserContext *UCCopy(UserContext *uc_in, void *new_pcb_p, void *unused) {
  pcb_t *new_pcb = (pcb_t *)new_pcb_p;
  int txt_page = mem_ctx->txt_start_page;
  int data_page = mem_ctx->data_start_page;
  int brk_page = mem_ctx->curr_brk_page;
  int stack_page = DOWN_TO_PAGE(uc_in->sp);
  void* brk_addr = (void*) (brk_page << PAGESHIFT);

  if (brk_page >= stack_page) {
    return NULL;
  }

  for (int vpn = txt_page; vpn < data_page; vpn++) {
    alloc_page(brk_page, PROT_READ | PROT_WRITE);
    memcpy(brk_addr, (void*) (long) (vpn << PAGESHIFT), PAGESIZE);
    int pfn = unmap_no_free(brk_page);

    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].pfn = pfn;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].valid = 1;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].prot = PROT_READ | PROT_EXEC;
  }

  for (int vpn = data_page; vpn < brk_page; vpn++) {
    if (!alloc_page(brk_page, PROT_READ | PROT_WRITE)) {
      return NULL;
    }
    memcpy(brk_addr, (void*) (vpn << PAGESHIFT), PAGESIZE);
    int pfn = unmap_no_free(brk_page);

    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].pfn = pfn;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].valid = 1;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].prot = PROT_READ | PROT_WRITE;
  }

  for (int vpn = stack_page; vpn < REGION1_VPNS; vpn++) {
    alloc_page(brk_page, PROT_READ | PROT_WRITE);
    memcpy(brk_addr, (void*) (long) (vpn << PAGESHIFT), PAGESIZE);
    int pfn = unmap_no_free(brk_page);

    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].pfn = pfn;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].valid = 1;
    new_pcb->mem_ctx.region1_pt[vpn - REGION1_BASE_VPN].prot = PROT_READ | PROT_WRITE;
  }

  new_pcb->mem_ctx.txt_start_page = txt_page;
  new_pcb->mem_ctx.data_start_page = data_page;
  new_pcb->mem_ctx.curr_brk_page = brk_page;

  memcpy(&(new_pcb->uc), uc_in, sizeof(UserContext));
  return uc_in;
}

/* Allocate a range of virtual pages with the requested protection. */
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

/* Switch to a new process's region 1 page table during context switch. */
void UCSwitch(UserContext *uc_in, void* curr_pcb_p, void* next_pcb_p) {
  pcb_t* curr_pcb = (pcb_t*) curr_pcb_p;
  pcb_t* next_pcb = (pcb_t*) next_pcb_p;

  mem_ctx = &(next_pcb->mem_ctx);

  WriteRegister(REG_PTBR1, (unsigned int) (long) mem_ctx->region1_pt);
  WriteRegister(REG_PTLR1, REGION1_VPNS);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
}

/* Allocate a single region 1 page and create its page table entry. */
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

/* Deallocate a region 1 page and free its physical frame. */
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

/* Change protection for a single region 1 page and flush the TLB. */
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

/* Unmap a region 1 page without freeing its frame, returning the PFN. */
static int unmap_no_free(int vpn) {
  int relative_vpn = vpn - REGION1_BASE_VPN;
  if (relative_vpn < REGION1_VPNS && relative_vpn >= 0) {
    int pfn = destroy_pte(mem_ctx->region1_pt, relative_vpn);
    WriteRegister(REG_TLB_FLUSH, vpn << PAGESHIFT);
    return pfn;
  }

  return -1;
}

/* Deallocate a range of region 1 pages. */
void undo_allocation(int first_vpn, int last_vpn) {
  for (int vpn = first_vpn; vpn < last_vpn; vpn++)
    dealloc_page(vpn);
}
