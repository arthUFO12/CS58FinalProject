#pragma once
#include "bool.h"
#include "hardware.h"

typedef int pid_t;

enum process_state { RUNNING, READY, WAITING, EXITED };

typedef struct pcb pcb_t;
typedef struct {
  pte_t *region1_pt;
  int curr_brk_page;
  int orig_brk_page;
  int txt_start_page;
  int data_start_page;
} mem_ctx_t;

typedef struct {
  bool has_parent;
  pcb_t **children;
  int num_children;
  int arr_size;
} rel_t;

struct pcb {
  KernelContext kc;
  UserContext uc;
  pid_t pid;
  enum process_state state;
  rel_t relations;
  int exit_code;

  int wake_up;
  int brk_page;
  pcb_t *next;
  pte_t *ks_pt;
  mem_ctx_t mem_ctx;
};

pcb_t *create_new_pcb(int num_ks_pages, int num_region1_pte, UserContext *uc);

void retire_pcb(pcb_t *pcb);

bool find_exited_child(pcb_t *parent, int *output);

bool add_child_proc(pcb_t *parent, pcb_t *child);



