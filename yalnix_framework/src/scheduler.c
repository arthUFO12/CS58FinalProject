/**
 * @file scheduler.c
 * @brief Determine the order and timing of user process execution
 */
#include "scheduler.h"
#include "bool.h"
#include "ylib.h"

#define HEAP_INIT_SIZE 20
#define HEAP_MIN_SIZE 5

typedef struct {
  pcb_t **arr;
  int length;
  int current_size;
} heap_t;

typedef struct {
  pcb_t *first;
  pcb_t *last;
  int length;
} queue_t;

static heap_t sleepers;
static int num_ticks;

static pcb_t *running_proc;

static queue_t queues[2];

enum queue_type { MAIN_QUEUE, BLOCKING_QUEUE };

/* Helper Definitions */
static pcb_t *deque_process();
static pcb_t *find_blocked_process(pid_t pid);
static void enque_process(enum queue_type type, pcb_t *pcb);
static bool resize_heap(int size);
static bool compare(pcb_t *a, pcb_t *b);
static bool add_to_heap(pcb_t *p);
static pcb_t *remove_from_heap(void);

pcb_t *get_running_proc() { return running_proc; }

void set_running_proc(pcb_t *new_proc) { running_proc = new_proc; }

void increment_ticks() { num_ticks++; }

bool init_scheduler(pcb_t *idle_proc) {
  /*
  sleepers.arr = calloc(HEAP_INIT_SIZE, sizeof(pcb_t *));
  sleepers.length = 0;
  sleepers.current_size = HEAP_INIT_SIZE;

  memset(queues, 0x00, sizeof(queues));
  running_proc = idle_proc;

  if (sleepers.arr == NULL)
    return false;

  return true;
  */
}

pcb_t *put_to_sleep(pcb_t *proc, int t) {
  /*
  proc->wake_up = num_ticks + t;
  if (queues[MAIN_QUEUE].length == 0 || !add_to_heap(proc)) {
    return proc;
  }

  pcb_t *new_proc = deque_process();

  proc->state = WAITING;
  new_proc->state = RUNNING;

  return new_proc;
  */
}

void wake_sleepers() {
  /*

  while (sleepers.length > 0 && sleepers.arr[0]->wake_up <= num_ticks) {
    enque_process(MAIN_QUEUE, remove_from_heap());
  }
}

void schedule_process(pcb_t *new_proc) {
  new_proc->state = READY;
  enque_process(MAIN_QUEUE, new_proc);
}

void block_process(pcb_t *new_proc) {
  new_proc->state = WAITING;
  enque_process(BLOCKING_QUEUE, new_proc);
}

pcb_t *run_diff_process(pcb_t *proc) {

  if (queues[MAIN_QUEUE].length == 0) {
    return proc;
  }

  pcb_t *new_proc = deque_process();

  enque_process(MAIN_QUEUE, proc);

  proc->state = READY;
  new_proc->state = RUNNING;

  return new_proc;
  */
}
