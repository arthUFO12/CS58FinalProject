#include "scheduler.h"
#include "bool.h"
#include "ylib.h"

#define HEAP_INIT_SIZE 20
#define HEAP_MIN_SIZE 5

typedef struct {
  pcb_t** arr;
  int length;
  int current_size;
} heap_t;

static heap_t sleepers;
static int num_ticks;

static pcb_t* running_proc;

typedef struct {
  pcb_t *first;
  pcb_t *last;
  int length;
} queue_t;

enum queue_type { MAIN_QUEUE, BLOCKING_QUEUE };

static queue_t queues[2];



static pcb_t *deque_process();
static pcb_t* find_blocked_process(pid_t pid);
static void enque_process(enum queue_type type, pcb_t *pcb);
static bool resize_heap(int size);
static bool compare(pcb_t* a, pcb_t* b);
static bool add_to_heap(pcb_t* p);
static pcb_t* remove_from_heap(void);


pcb_t* get_running_proc() {
  return running_proc;
}

void set_running_proc(pcb_t* new_proc) {
  running_proc = new_proc;
}

void increment_ticks() {
  num_ticks++;
}


bool init_scheduler(pcb_t* idle_proc) {
  sleepers.arr = calloc(HEAP_INIT_SIZE, sizeof(pcb_t*));
  sleepers.length = 0;
  sleepers.current_size = HEAP_INIT_SIZE;

  memset(queues, 0x00, sizeof(queues));
  running_proc = idle_proc;

  if (sleepers.arr == NULL) return false;

  return true;
}

static bool resize_heap(int size) {
  pcb_t** temp = calloc(size, sizeof(pcb_t*));

  if (temp == NULL) return false;

  memcpy(temp, sleepers.arr, sleepers.length * sizeof(pcb_t*));

  free(sleepers.arr);
  sleepers.arr = temp;
  sleepers.current_size = size;

  return true;
}

static bool compare(pcb_t* a, pcb_t* b) {
  return a->wake_up < b->wake_up;
}

static bool add_to_heap(pcb_t* p) {
  if (sleepers.length >= sleepers.current_size 
      && !resize_heap(sleepers.current_size * 2)) {
    return false;
  }

  int i = sleepers.length;
  sleepers.arr[i] = p;
  sleepers.length++;

  while (i > 0) {
    int parent = (i - 1) / 2;

    if (compare(sleepers.arr[parent], sleepers.arr[i])) {
      break;
    }

    pcb_t* temp = sleepers.arr[parent];
    sleepers.arr[parent] = sleepers.arr[i];
    sleepers.arr[i] = temp;

    i = parent;
  }

  return true;
}

static pcb_t* remove_from_heap(void) {
  if (sleepers.length == 0) {
    return NULL;
  }
  
  pcb_t* to_return = sleepers.arr[0];

  int i = 0;
  sleepers.arr[i] = sleepers.arr[--sleepers.length];


  while (true) {
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    int least = i;

    if (left < sleepers.length && compare(sleepers.arr[left], sleepers.arr[least])) {
      least = left;
    }

    if (right < sleepers.length && compare(sleepers.arr[right], sleepers.arr[least])) {
      least = right;
    }

    if (least == i) {
      break;
    }

    pcb_t* temp = sleepers.arr[least];
    sleepers.arr[least] = sleepers.arr[i];
    sleepers.arr[i] = temp;

    i = least;
  }

  if (sleepers.length < sleepers.current_size / 2 && sleepers.current_size > HEAP_MIN_SIZE) {
    resize_heap(sleepers.current_size / 2);
  }

  return to_return;
}


pcb_t* put_to_sleep(pcb_t* proc, int t) {
  proc->wake_up = num_ticks + t;
  if (queues[MAIN_QUEUE].length == 0 || !add_to_heap(proc)) {
    return proc;
  }

  pcb_t* new_proc = deque_process();
  
  proc->state = WAITING;
  new_proc->state = RUNNING;

  return new_proc;
}

void wake_sleepers() {

  while (sleepers.length > 0 && sleepers.arr[0]->wake_up <= num_ticks) {
    schedule_process(remove_from_heap());
  }

}
void schedule_process(pcb_t* new_proc) {
  new_proc->state = READY;
  enque_process(MAIN_QUEUE, new_proc);
}

void block_process(pcb_t* new_proc) {
  new_proc->state = WAITING;
  enque_process(BLOCKING_QUEUE, new_proc);
}

pcb_t* run_diff_process(pcb_t * proc) {

  if (queues[MAIN_QUEUE].length == 0) {
    return proc;
  }

  pcb_t* new_proc = deque_process();

  enque_process(MAIN_QUEUE, proc);

  proc->state = READY;
  new_proc->state = RUNNING;

  return new_proc;
}

static void enque_process(enum queue_type type, pcb_t *pcb) {
  if (queues[type].length == 0) {
    queues[type].first = pcb;
    queues[type].last = pcb;
  } else {
    pcb_t *last = queues[type].last;
    last->next = pcb;
    queues[type].last = pcb;
  }

  pcb->next = NULL;
  queues[type].length++;

  TracePrintf(0, "Enqued pcb with pid %d\nSize is now %d\nFirst in line is %d\n", pcb->pid, queues[type].length, queues[type].first->pid);
}

static pcb_t *deque_process() {

  if (queues[MAIN_QUEUE].length == 0)
    return NULL;

  pcb_t *popped = queues[MAIN_QUEUE].first;
  queues[MAIN_QUEUE].first = popped->next;
  queues[MAIN_QUEUE].length--;

  if (queues[MAIN_QUEUE].length == 0)
    queues[MAIN_QUEUE].last = NULL;

  popped->next = NULL;

  return popped;
}

static pcb_t* find_blocked_process(pid_t pid) {

  pcb_t* prev = NULL;
  pcb_t* curr = queues[BLOCKING_QUEUE].first;

  while (curr != NULL) {

    if (curr->pid == pid) {
      if (prev == NULL) {
        queues[BLOCKING_QUEUE].first = curr->next;
      }
      else {
        prev->next = curr->next;
      }

      if (queues[BLOCKING_QUEUE].last == curr) {
        queues[BLOCKING_QUEUE].last = prev;
      }

      queues[BLOCKING_QUEUE].length--;

      curr->next = NULL;

      return curr;
    }

    prev = curr;
    curr = curr->next;
  }

  return NULL;
}