
#pragma once
#include "bool.h"
#include "hardware.h"

/*
 * Frame allocator and page table entry helper APIs.
 * These routines track physical page frames and provide
 * basic helpers for creating and destroying PTE entries.
 */
bool acquire_frame(int vpn);
bool initialize_frame_tracking(int pmem_size);
int find_frame(void);
bool free_frame(int frame_num);

/*
 * Page table entry helpers. destroy_pte clears an entry and
 * returns the old frame number; create_pte initializes the
 * entry with a frame number and protection bits.
 */
int destroy_pte(pte_t *base, int vpn);
void create_pte(pte_t *base, int vpn, int pfn, int prot);

