#include "bool.h"
#include "hardware.h"
#include "pcb.h"

#define REGION0_VPNS (VMEM_0_SIZE >> PAGESHIFT)
#define REGION1_VPNS (VMEM_1_SIZE >> PAGESHIFT)
#define NUM_K_STACK_VPNS (KERNEL_STACK_MAXSIZE >> PAGESHIFT)
#define K_STACK_BASE_VPN (KERNEL_STACK_BASE >> PAGESHIFT)
#define K_STACK_LIMIT_VPN (KERNEL_STACK_LIMIT >> PAGESHIFT)
#define K_STACK_NUM_VPN (K_STACK_LIMIT_VPN - K_STACK_BASE_VPN)

/* Enable virtual memory in the hardware. */
void enable_vm(void);

/* Initialize the kernel's Region 0 page table using the idle PCB. */
bool init_region0_pt(pcb_t* idle_pcb);

/*
 * Kernel stack context switch callback.
 * Saves the current kernel context and prepares the next PCB.
 */
KernelContext *KCSwitch(KernelContext *kc_in, void *curr_pcb_p,
                        void *next_pcb_p);

/*
 * Kernel context copy callback used when creating a new process.
 * Copies kernel stack and context into the new PCB.
 */
KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *unused);

/* Initialize kernel break pointer state before user programs start. */
void init_kernel_brk(void);

