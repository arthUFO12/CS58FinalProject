
#include "frame_tracking.h"
#include "ylib.h"

/*
 * Frame tracking implementation for physical memory allocation.
 * This module uses a bitset to track allocated physical frames and
 * provides helper functions for page table entry management.
 */

#define BYTE_SIZE 8
#define BIT_OFFSET (BYTE_SIZE - 1)
#define BYTE_MASK (~BYTE_SIZE)
#define BYTE_SHIFT 3
#define UP_TO_BYTE(n) (((long)(n) + BIT_OFFSET) & BYTE_MASK)
#define FULL_BYTE 0xffu

#define PMEM_BASE_FRAME (DOWN_TO_PAGE(PMEM_BASE) >> PAGESHIFT)

typedef unsigned char byte_t;
static byte_t *frame_bitset;
static int next_free;
static int NUM_FRAMES;

/* Initialize the frame allocator for the given physical memory size. */
bool initialize_frame_tracking(int pmem_size) {
  TracePrintf(0, "Starting frame tracking initialization\n");
  int pmem_frame_limit = DOWN_TO_PAGE(pmem_size) >> PAGESHIFT;
  NUM_FRAMES = pmem_frame_limit - PMEM_BASE_FRAME;
  int bitset_size = UP_TO_BYTE(NUM_FRAMES) >> BYTE_SHIFT;
  frame_bitset = calloc(bitset_size, sizeof(byte_t));

  next_free = 0;
  TracePrintf(0, "Frame bitset location: %p\n", frame_bitset);

  return frame_bitset != NULL;
}

/* Convert a relative frame number into an absolute physical frame number. */
static int add_base(int frame_num) { return frame_num + PMEM_BASE_FRAME; }

/* Convert an absolute physical frame number into a relative frame index. */
static int subtract_base(int frame_num) { return frame_num - PMEM_BASE_FRAME; }

/* Return true when the frame at the given index is free. */
static bool check_if_free(int frame_num) {
  int byte_num = frame_num >> BYTE_SHIFT;

  if (frame_bitset[byte_num] == FULL_BYTE)
    return false;

  int bit_offset = frame_num & BIT_OFFSET;
  byte_t mask = 1 << bit_offset;
  byte_t entry = frame_bitset[byte_num] & mask;

  return entry == 0;
}

/* Mark a bitset frame index as allocated. */
static void set_frame(int frame_num) {
  int byte_num = frame_num >> BYTE_SHIFT;
  int bit_offset = frame_num & BIT_OFFSET;
  byte_t mask = 1 << bit_offset;

  frame_bitset[byte_num] |= mask;
  TracePrintf(0, "Frame 0x%x is now allocated\n", frame_num);
}

/* Clear the allocation bit for the given frame index. */
static void clear_frame(int frame_num) {
  int byte_num = frame_num >> BYTE_SHIFT;
  int bit_offset = frame_num & BIT_OFFSET;
  byte_t mask = ~(1 << bit_offset);

  frame_bitset[byte_num] &= mask;
  TracePrintf(0, "Frame 0x%3x is now deallocated\n", frame_num);
}

/* Find the next free frame and allocate it, returning its PFN. */
int find_frame(void) {
  while (!check_if_free(next_free) && next_free < NUM_FRAMES)
    next_free++;

  if (next_free == NUM_FRAMES)
    return ERROR;

  set_frame(next_free);
  return add_base(next_free++);
}

/* Allocate a specific physical frame number if it is free. */
bool acquire_frame(int pfn) {
  pfn = subtract_base(pfn);

  if (!check_if_free(pfn))
    return false;

  set_frame(pfn);
  return true;
}

/* Free the specified physical frame and update the free search pointer. */
bool free_frame(int frame_num) {
  frame_num = subtract_base(frame_num);

  if (frame_num < 0 || frame_num >= NUM_FRAMES)
    return false;
  if (check_if_free(frame_num))
    return false;

  clear_frame(frame_num);
  next_free = (frame_num < next_free) ? frame_num : next_free;
  return true;
}

/* Invalidate a page table entry and return the former physical frame. */
int destroy_pte(pte_t *base, int vpn) {
  base[vpn].valid = 0;
  return base[vpn].pfn;
}

/* Create or update a page table entry with the given frame and protection. */
void create_pte(pte_t *base, int vpn, int pfn, int prot) {
  base[vpn].valid = 1;
  base[vpn].pfn = pfn;
  base[vpn].prot = prot;
}