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

  /*

  // Create bitmap
  number_available_frames = total frames - base frames;
  frame_bitset = calloc(bitset_size, sizeof(byte_t));

  next_free = 0;


  return status;
  */
}

/* Helper Definitions */
static int add_base(int frame_num);

static int subtract_base(int frame_num);

static bool check_if_free(int frame_num);

static void set_frame(int frame_num);

static void clear_frame(int frame_num);

int find_frame(void);

bool acquire_frame(int pfn);

bool free_frame(int frame_num);

int destroy_pte(pte_t *base, int vpn) {
  base[vpn].valid = 0;
  return base[vpn].pfn;
}

void create_pte(pte_t *base, int vpn, int pfn, int prot) {
  base[vpn].valid = 1;
  base[vpn].pfn = pfn;
  base[vpn].prot = prot;
}
