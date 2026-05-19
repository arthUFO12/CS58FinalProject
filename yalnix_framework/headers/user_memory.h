
#include "bool.h"
#include "hardware.h"
#include "pcb.h"

#ifndef REGION0_VPNS
#define REGION0_VPNS (VMEM_0_SIZE >> PAGESHIFT)
#endif

#ifndef REGION1_VPNS
#define REGION1_VPNS (VMEM_1_SIZE >> PAGESHIFT)
#endif

#define REGION1_BASE_VPN REGION0_VPNS
#define REGION1_LIMIT_VPN (REGION1_BASE_VPN + REGION1_VPNS)

int KernelBrk_Impl(void *addr, void *sp);

bool init_region1_pt(pcb_t *idle);

void UCSwitch(pcb_t *next_pcb);

bool UCCopy(UserContext *uc_in, pcb_t *new_pcb);

void deallocate_region1(void);

bool alloc_region(int start_vpn, int end_vpn, int prot);

void prot_region(int start_vpn, int end_vpn, int prot);
