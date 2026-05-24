#pragma once
#include "hardware.h"

/* Process identifier type. */
typedef int pid_t;

enum process_state {
  RUNNING,
  READY,
  WAITING
};

typedef struct pcb pcb_t;

/* Memory context used by a process to manage its Region 1 mapping. */
typedef struct {
  pte_t* region1_pt;        /* Region 1 page table base. */
  int curr_brk_page;        /* Current heap break page. */
  int orig_brk_page;        /* Original brk page on program load. */
  int txt_start_page;       /* Start page of the text segment. */
  int data_start_page;      /* Start page of the data segment. */
} mem_ctx_t;


struct pcb {
  KernelContext kc;         /* Saved kernel context for context switch. */
  UserContext uc;           /* User context for the process. */
  pid_t pid;                /* Process identifier. */
  enum process_state state; /* Current scheduler state. */

  int wake_up;              /* Tick count when a sleeping process should wake. */
  int brk_page;             /* Kernel break page for kernel heap management. */
  pcb_t *next;              /* Linked list pointer used for queues. */
  pte_t *ks_pt;             /* Kernel stack page table for this PCB. */
  mem_ctx_t mem_ctx;        /* Process memory context. */

};


/* Allocate and initialize a new process control block. */
pcb_t *create_new_pcb(int num_ks_pages, int num_region1_pte, UserContext *uc);
