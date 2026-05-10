
#include "bool.h"
#include "hardware.h"

bool acquire_frame(int vpn);
bool initialize_frame_tracking(int pmem_size);
int find_frame(void);
bool free_frame(int frame_num);
int destroy_pte(pte_t *base, int vpn);
void create_pte(pte_t *base, int vpn, int pfn, int prot);
