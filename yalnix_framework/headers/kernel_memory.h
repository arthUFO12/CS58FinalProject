#pragma once
#include "bool.h"
#include "hardware.h"
#include "pcb.h"

#ifndef REGION0_VPNS
#define REGION0_VPNS (VMEM_0_SIZE >> PAGESHIFT)
#endif

#ifndef REGION1_VPNS
#define REGION1_VPNS (VMEM_1_SIZE >> PAGESHIFT)
#endif

#define K_STACK_BASE_VPN (KERNEL_STACK_BASE >> PAGESHIFT)
#define K_STACK_LIMIT_VPN (KERNEL_STACK_LIMIT >> PAGESHIFT)
#define K_STACK_VPNS (K_STACK_LIMIT_VPN - K_STACK_BASE_VPN)

/* Enable the hardware virtual memory unit. */
void enable_vm(void);

/* Initialize the kernel region 0 page table for the idle process. */
bool init_region0_pt(pcb_t *idle_pcb);

/* Kernel context switch callback used by the interrupt subsystem. */
KernelContext *KCSwitch(KernelContext *kc_in, void *curr_pcb_p,
                        void *next_pcb_p);

/* Kernel context copy callback used for process fork/cloning. */
KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *unused);

/* Initialize the kernel heap break point for the kernel address space. */
void init_kernel_brk(void);

/* Free the kernel stack pages for a terminated process. */
void deallocate_kernel_stack(void);
