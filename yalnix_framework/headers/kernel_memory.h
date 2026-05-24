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

void enable_vm(void);

bool init_region0_pt(pcb_t *idle_pcb);

KernelContext *KCSwitch(KernelContext *kc_in, void *curr_pcb_p,
                        void *next_pcb_p);

KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *unused);

void init_kernel_brk(void);

void deallocate_kernel_stack(void);
