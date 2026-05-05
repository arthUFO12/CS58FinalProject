
#include "PCB.h"
#include "ylib.h"

typedef struct {
  PCB_t* first;
  PCB_t* last;
  int size;
} queue_t;

static queue_t queues[2];

void initPCBQueue() {
  memset(queues, 0x00, sizeof(queues));
}

void enquePCB(enum queue_type type, PCB_t* pcb) {
  if (queues[type].size == 0) {
    queues[type].first = pcb;
    queues[type].last = pcb;
  }
  else {
    PCB_t* last = queues[type].last;
    last->next = pcb;
    queues[type].last = pcb;
  }

  pcb->next = NULL;
  queues[type].size++;
}

PCB_t* dequePCB(enum queue_type type) {

  if (queues[type].size == 0) return NULL;

  PCB_t* popped = queues[type].first;
  queues[type].first = popped->next;
  queues[type].size--;

  if (queues[type].size == 0) queues[type].last = NULL;

  popped->next = NULL;
  
  return popped;

}

