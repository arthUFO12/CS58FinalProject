
#include "hardware.h"


typedef int pid_t;


typedef struct {
  KernelContext kc;
  UserContext uc;
  pid_t pid;
  enum { RUNNING, READY, BLOCKED } state;
  PCB_t* next;
  pte_t* page_table;

} PCB_t;


enum queue_type {
  MAIN_QUEUE,
  BLOCKING_QUEUE
};

void initPCBQueue();

PCB_t* dequePCB(enum queue_type q);

void enquePCB(enum queue_type q, PCB_t* pcb);

