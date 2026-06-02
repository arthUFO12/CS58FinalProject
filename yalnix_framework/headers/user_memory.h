
#pragma once
#include "bool.h"
#include "hardware.h"
#include "pcb.h"

/*
 * Region 1 layout definitions describe the user-space virtual page
 * number range used for process text, data, heap, and stack.
 */
#ifndef REGION0_VPNS
#define REGION0_VPNS (VMEM_0_SIZE >> PAGESHIFT)
#endif

#ifndef REGION1_VPNS
#define REGION1_VPNS (VMEM_1_SIZE >> PAGESHIFT)
#endif

#define REGION1_BASE_VPN REGION0_VPNS
#define REGION1_LIMIT_VPN (REGION1_BASE_VPN + REGION1_VPNS)

/*
 * KernelBrk_Impl adjusts the current process heap boundary for region 1.
 * `addr` is the new heap end, and `sp` is the current user stack pointer.
 */
int KernelBrk_Impl(void *addr, void *sp);

/* Initialize the region 1 page table for the idle process. */
bool init_region1_pt(pcb_t *idle);

/* Switch the active region 1 page table to the next process. */
void UCSwitch(pcb_t *next_pcb);

/* Copy user address space and register state for forked processes. */
bool UCCopy(UserContext *uc_in, pcb_t *new_pcb);

/* Release all region 1 pages for the current process. */
void deallocate_region1(void);

/* Allocate a contiguous range of region 1 virtual pages with protection. */
bool alloc_region(int start_vpn, int end_vpn, int prot);

/* Change protection on a range of region 1 virtual pages. */
void prot_region(int start_vpn, int end_vpn, int prot);


bool expand_stack(int sp);


