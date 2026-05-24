
#include "hardware.h"
#include "pcb.h"
#include "bool.h"

/*
 * User memory management helpers for Region 1 virtual memory.
 * Handles allocation, protection, and page table switching.
 */

/* Adjust the program break for the current user heap area. */
int KernelBrk_Impl(void* addr, void* sp);

/* Initialize the Region 1 page table for the idle process. */
bool init_region1_pt(pcb_t* idle);

/* Switch Region 1 page tables for a context switch. */
void UCSwitch(UserContext *uc_in, void* curr_pcb_p, void* next_pcb_p);

/* Deallocate a range of virtual pages in Region 1. */
void undo_allocation(int first_vpn, int last_vpn);

/* Allocate a range of virtual pages with the given protection. */
bool alloc_region(int start_vpn, int end_vpn, int prot);

/* Change protection bits for a range of virtual pages. */
void prot_region(int start_vpn, int end_vpn, int prot);
