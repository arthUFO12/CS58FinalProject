#include "bool.h"
#include "hardware.h"

#define REGION0_VPNS (VMEM_0_SIZE >> PAGESHIFT)
#define REGION1_VPNS (VMEM_1_SIZE >> PAGESHIFT)
#define NUM_K_STACK_VPNS (KERNEL_STACK_MAXSIZE >> PAGESHIFT)
#define K_STACK_BASE_VPN (KERNEL_STACK_BASE >> PAGESHIFT)

void enable_vm(void);

bool setup_region0_pt();

bool setup_region1_pt(void);

KernelContext *KCSwitch(KernelContext *kc_in, void *curr_pcb_p,
                        void *next_pcb_p);

KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *unused);
