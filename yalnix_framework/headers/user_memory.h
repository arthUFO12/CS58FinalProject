
#include "bool.h"
#include "hardware.h"
#include "pcb.h"

int KernelBrk_Impl(void *addr, void *sp);

bool init_region1_pt(pcb_t *idle);

void UCSwitch(void *next_pcb_p);

bool UCCopy(UserContext *uc_in, pcb_t *new_pcb);

void deallocate_region1(void);

bool alloc_region(int start_vpn, int end_vpn, int prot);

void prot_region(int start_vpn, int end_vpn, int prot);
