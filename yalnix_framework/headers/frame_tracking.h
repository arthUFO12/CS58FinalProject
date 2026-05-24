
#include "bool.h"
#include "hardware.h"

/*
 * Physical frame allocation and page table manipulation helpers.
 * This module tracks free physical frames and allows the kernel
 * to manage page table entries.
 */

/* Mark a physical frame as in use. */
bool acquire_frame(int vpn);

/* Initialize the frame allocator for pmem_size bytes of physical memory. */
bool initialize_frame_tracking(int pmem_size);

/* Find a free physical frame and allocate it. */
int find_frame(void);

/* Free the given physical frame. */
bool free_frame(int frame_num);

/* Destroy a page table entry and return its frame number. */
int destroy_pte(pte_t *base, int vpn);

/* Create or update a page table entry for the specified VPN. */
void create_pte(pte_t *base, int vpn, int pfn, int prot);
