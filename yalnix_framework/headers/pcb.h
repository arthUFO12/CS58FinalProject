#pragma once
#include "bool.h"
#include "hardware.h"

/* Process identifier type. */
typedef int pid_t;

/* Process life-cycle states supported by the scheduler. */
enum process_state { RUNNING, READY, WAITING, EXITED };

typedef struct pcb pcb_t;

/* Memory context for a process's region 1 address space. */
typedef struct {
  pte_t *region1_pt;   /* Region 1 page table base pointer. */
  int curr_brk_page;   /* Current heap break page number. */
  int orig_brk_page;   /* Initial heap break page number after exec. */
  int txt_start_page;  /* Region 1 virtual page where text begins. */
  int data_start_page; /* Region 1 virtual page where data begins. */
} mem_ctx_t;

/* Parent / child relationships for process wait semantics. */
typedef struct {
  bool has_parent;
  pcb_t **children;
  int num_children;
  int arr_size;
} rel_t;

/* Process control block structure. */
struct pcb {
  KernelContext kc; /* Kernel registers and stack context. */
  UserContext uc;   /* User register state for this process. */
  pid_t pid;        /* Unique process identifier. */
  enum process_state state;
  rel_t relations; /* Parent/child tracking information. */
  int exit_code;   /* Exit code returned by the process. */

  int wake_up;       /* Clock tick when a sleeping process should resume. */
  int brk_page;      /* Process heap boundary page, if applicable. */
  pcb_t *next;       /* Next PCB in scheduler or wait queues. */
  pte_t *ks_pt;      /* Kernel stack page table entries for this process. */
  mem_ctx_t mem_ctx; /* User-space memory context for region 1. */
  int cvar_lock_id;

  char *io_kbuf;     /* Region 0 buffer for pending write payload. */
  void *io_user_buf; /* User pointer for pending read destination. */
  int io_total;      /* Total bytes for current write request. */
  int io_sent;       /* Bytes already submitted via TtyTransmit. */
  int io_len;        /* Requested bytes for current read request. */
  int io_tty_id;     /* Terminal id of pending I/O operation. */
};

/* Create a new process control block with reserved kernel stack and region 1 pages. */
pcb_t *create_new_pcb(int num_ks_pages, int num_region1_pte, UserContext *uc);

/* Clean up process resources when a process terminates. */
void retire_pcb(pcb_t *pcb);

/* Locate an exited child process and optionally return its exit code. */
bool find_exited_child(pcb_t *parent, int *output);

/* Add a child PCB to a parent's child list. */
bool add_child_proc(pcb_t *parent, pcb_t *child);
