/**
 * @file frame_tracking.c
 * @brief Creates bitmap and defines helpers to track frame status
 */
#include "frame_tracking.h"
#include "ylib.h"

#define BYTE_SIZE 8
#define BIT_OFFSET (BYTE_SIZE - 1)
#define BYTE_MASK (~BIT_OFFSET)
#define BYTE_SHIFT 3
#define UP_TO_BYTE(n) (((long)(n) + BIT_OFFSET) & BYTE_MASK)
#define FULL_BYTE 0xffu
#define PMEM_BASE_FRAME (DOWN_TO_PAGE(PMEM_BASE) >> PAGESHIFT)

typedef unsigned char byte_t;
static int next_free;
static int NUM_FRAMES;

/** Bitmap tracking frame status */
static byte_t *frame_bitset;

/**
 * @brief Create bitmap to track frame status
 *
 * @param pmem_size Physical memory size
 * @return status
 */
bool initialize_frame_tracking(int pmem_size) {

  TracePrintf(0, "Starting frame tracking initialization\n");

  // Create bitmap
  int pmem_frame_limit = DOWN_TO_PAGE(pmem_size) >> PAGESHIFT; // get total number of frames
  NUM_FRAMES = pmem_frame_limit - PMEM_BASE_FRAME;
  int bitset_size = UP_TO_BYTE(NUM_FRAMES) >> BYTE_SHIFT;
  frame_bitset = calloc(bitset_size, sizeof(byte_t));

  next_free = 0;

  TracePrintf(0, "Frame bitset location: %p\n", frame_bitset);

  if (frame_bitset != NULL) {
    return true;
  }

  return false;
}

/* Helpers */
static int add_base(int frame_num) { return frame_num + PMEM_BASE_FRAME; }

static int subtract_base(int frame_num) { return frame_num - PMEM_BASE_FRAME; }

static bool check_if_free(int frame_num) {
  int byte_num = frame_num >> BYTE_SHIFT;

  if (frame_bitset[byte_num] == FULL_BYTE)
    return false;

  int bit_offset = frame_num & BIT_OFFSET;
  byte_t mask = 1 << bit_offset;

  byte_t entry = frame_bitset[byte_num] & mask;

  if (entry == 0)
    return true;
  else
    return false;
}

static void set_frame(int frame_num) {
  int byte_num = frame_num >> BYTE_SHIFT;
  int bit_offset = frame_num & BIT_OFFSET;
  byte_t mask = 1 << bit_offset;

  frame_bitset[byte_num] |= mask;

  TracePrintf(0, "Frame 0x%x is now allocated\n", frame_num);
}

static void clear_frame(int frame_num) {
  int byte_num = frame_num >> BYTE_SHIFT;
  int bit_offset = frame_num & BIT_OFFSET;
  byte_t mask = ~(1 << bit_offset);

  frame_bitset[byte_num] &= mask;

  TracePrintf(0, "Frame 0x%3x is now deallocated\n", frame_num);
}

int find_frame(void) {

  while (!check_if_free(next_free) && next_free < NUM_FRAMES)
    next_free++;

  if (next_free == NUM_FRAMES)
    return ERROR;

  set_frame(next_free);

  return add_base(next_free++);
}

bool acquire_frame(int pfn) {
  pfn = subtract_base(pfn);

  if (!check_if_free(pfn))
    return false;

  set_frame(pfn);

  return true;
}

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

int destroy_pte(pte_t *base, int vpn) {
  base[vpn].valid = 0;
  return base[vpn].pfn;
}

void create_pte(pte_t *base, int vpn, int pfn, int prot) {
  base[vpn].valid = 1;
  base[vpn].pfn = pfn;
  base[vpn].prot = prot;
}
