
#include "pcb.h"
#include "ykernel.h"
#include "ylib.h"

typedef struct {
  pcb_t *first;
  pcb_t *last;
  int size;
} queue_t;

static queue_t queues[2];

void init_pcb_queue() { memset(queues, 0x00, sizeof(queues)); }

void enque_pcb(enum queue_type type, pcb_t *pcb) {
  if (queues[type].size == 0) {
    queues[type].first = pcb;
    queues[type].last = pcb;
  } else {
    pcb_t *last = queues[type].last;
    last->next = pcb;
    queues[type].last = pcb;
  }

  pcb->next = NULL;
  queues[type].size++;
}

pcb_t *deque_pcb(enum queue_type type) {

  if (queues[type].size == 0)
    return NULL;

  pcb_t *popped = queues[type].first;
  queues[type].first = popped->next;
  queues[type].size--;

  if (queues[type].size == 0)
    queues[type].last = NULL;

  popped->next = NULL;

  return popped;
}

pcb_t *create_init_pcb(int num_ks_pages, int num_region1_pages,
                       UserContext *uc) {
  pcb_t *pcb = calloc(1, sizeof(pcb_t));

  if (pcb == NULL)
    return NULL;

  pcb->ks_pt = calloc(1, num_ks_pages * sizeof(pte_t));
  if (pcb->ks_pt == NULL)
    return NULL;

  pcb->region1_pt = calloc(1, num_region1_pages * sizeof(pte_t));
  if (pcb->region1_pt == NULL)
    return NULL;

  pcb->pid = helper_new_pid(pcb->region1_pt);
  pcb->state = RUNNING;

  memcpy(&(pcb->uc), uc, sizeof(UserContext));

  return pcb;
}

void set_up_uc(UserContext *uc, void (*idle_func)(void), int sp) {

  memset(uc, 0x00, sizeof(UserContext));
  uc->pc = idle_func;
  uc->sp = (void *)(long)sp;
}
