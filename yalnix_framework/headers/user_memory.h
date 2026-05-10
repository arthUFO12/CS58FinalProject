
#include "hardware.h"
#include "pcb.h"
#include "bool.h"

int KernelBrk_Impl(void* addr, void* sp);


bool init_region1_pt(pcb_t* idle);

void UCSwitch(UserContext *uc_in, void* curr_pcb_p, void* next_pcb_p);

void undo_allocation(int first_vpn, int last_vpn);

bool alloc_region(int start_vpn, int end_vpn, int prot);

void prot_region(int start_vpn, int end_vpn, int prot);
