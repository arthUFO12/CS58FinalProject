/**
 * @file user_memory.c
 * @brief Sets up and defines functions that control virtual memory for user
 */

#include "user_memory.h"
#include "frame_tracking.h"
#include "pcb.h"
#include "ylib.h"

#define REGION0_VPNS (VMEM_0_SIZE >> PAGESHIFT)
#define REGION1_VPNS (VMEM_1_SIZE >> PAGESHIFT)
#define REGION1_BASE_VPN REGION0_VPNS
#define REGION1_LIMIT_VPN (REGION1_BASE_VPN + REGION1_VPNS)

/** Memory context of currently running PCB */
static mem_ctx_t *mem_ctx;

/* Helper Definitions */
static bool alloc_page(int vpn, int prot);
static bool dealloc_page(int vpn);
static int unmap_no_free(int vpn);
static void change_prot(int vpn, int prot);

bool init_region1_pt(pcb_t *idle) {
  /*
  mem_ctx = &(idle->mem_ctx);

  TracePrintf(0, "Starting region 1 pt initialization\n");

  TracePrintf(0, "Writing region1 page table to register\n");

  WriteRegister(REG_PTBR1, (unsigned int)(long)mem_ctx->region1_pt);
  WriteRegister(REG_PTLR1, REGION1_VPNS);

  mem_ctx->txt_start_page = REGION1_BASE_VPN;
  mem_ctx->data_start_page = REGION1_BASE_VPN;
  mem_ctx->orig_brk_page = REGION1_BASE_VPN;
  mem_ctx->curr_brk_page = REGION1_BASE_VPN;

  return alloc_page(DOWN_TO_PAGE(VMEM_1_LIMIT - 1) >> PAGESHIFT, PROT_READ | PROT_WRITE);
  */
}

int KernelBrk_Impl(void *addr, void *sp) {
  /*
     convert addr to page index

     check if heap would intersect stack

     allocate or free pages

     update break pointer

     return status
    */
}

UserContext *UCCopy(UserContext *uc_in, void *new_pcb_p, void *unused) {
  /*
  save original boundaries and break

  create temp page to store ( increase break)

  copy text (read only + exec)
  copy data (read + write)
  copy stack (read + write)

  give page to new_pcb

  return original context
  */
}

bool alloc_region(int start_vpn, int end_vpn, int prot) {
  /*
  if (start_vpn < REGION1_BASE_VPN || end_vpn > REGION1_BASE_VPN + REGION1_VPNS) {
    return false;
  }

  for (int vpn = start_vpn; vpn < end_vpn; vpn++) {
    if (!alloc_page(vpn, prot)) {
      undo_allocation(start_vpn, vpn);
      return false;
    }
  }

  return true;
  */
}

void prot_region(int start_vpn, int end_vpn, int prot) {
  /*
  if (start_vpn < REGION1_BASE_VPN || end_vpn > REGION1_BASE_VPN + REGION1_VPNS) {
    return;
  }

  for (int vpn = start_vpn; vpn < end_vpn; vpn++) {
    change_prot(vpn, prot);
  }
  */
}

void UCSwitch(UserContext *uc_in, void *curr_pcb_p, void *next_pcb_p) {
  /*
  pcb_t *curr_pcb = (pcb_t *)curr_pcb_p;
  pcb_t *next_pcb = (pcb_t *)next_pcb_p;

  mem_ctx = &(next_pcb->mem_ctx);

  WriteRegister(REG_PTBR1, (unsigned int)(long)mem_ctx->region1_pt);
  WriteRegister(REG_PTLR1, REGION1_VPNS);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
  */
}

void undo_allocation(int first_vpn, int last_vpn) {
  /*
  for (int vpn = first_vpn; vpn < last_vpn; vpn++)
    dealloc_page(vpn);
  */
}
