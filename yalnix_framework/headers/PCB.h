
#include "hardware.h"

typedef int pid_t;

enum process_state { RUNNING, READY, BLOCKED };

typedef struct pcb pcb_t;

struct pcb {
  KernelContext kc;
  UserContext uc;
  pid_t pid;
  enum process_state state;

  pcb_t *next;
  pte_t *ks_pt;
  pte_t *region1_pt;
};

enum queue_type { MAIN_QUEUE, BLOCKING_QUEUE };

void init_pcb_queue();

pcb_t *deque_pcb(enum queue_type q);

void enque_pcb(enum queue_type q, pcb_t *pcb);

void set_up_uc(UserContext *uc, void (*idle_func)(void), int sp);

pcb_t *create_init_pcb(int num_ks_pages, int num_region1_pages,
                       UserContext *uc);
