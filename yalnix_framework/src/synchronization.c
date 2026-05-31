
#include "synchronization.h"
#include "bool.h"
#include "pcb.h"
#include "ylib.h"
#include "identifiers.h"
#include "scheduler.h"
#include "interrupt.h"

#ifndef ERROR
#define ERROR -1
#endif

#ifndef SUCCESS
#define SUCCESS 0 
#endif


#define ACQUIRED 0 
#define NEED -2

#define INITIAL_ARR_SIZE 10

typedef struct {
  bool acquired;
  bool in_use;
  pcb_t* first;
  pcb_t* last;
  pid_t pid;
  int needed_by;
} lock_t;



typedef struct {
  pcb_t* first;
  pcb_t* last;
  bool in_use;
} cvar_t;


lock_t* locks;
cvar_t* cvars;
int open_lock_idx = 0;
int open_cvar_idx = 0;
int cvar_arr_size, lock_arr_size;

static int new_cvar(void);
static int new_lock(void);
static bool delete_lock(int lock_id);
static bool delete_cvar(int cvar_id);

static void insert(pcb_t** first, pcb_t** last, pcb_t* new);
static pcb_t* pop(pcb_t** first, pcb_t** last);


static bool release(int lock_id, pcb_t* proc, bool cvar_use);
static int acquire(int lock_id, pcb_t* proc, bool cvar_use);
static bool wait_cvar(int cvar_id, int lock_id, pcb_t* proc);
static bool signal_cvar(int cvar_id);
static bool broadcast_cvar(int cvar_id);

bool init_sync(void) {


  if ((locks = calloc(INITIAL_ARR_SIZE, sizeof(lock_t))) == NULL) goto fail;

  if ((cvars = calloc(INITIAL_ARR_SIZE, sizeof(cvar_t))) == NULL) goto cleanup;


  cvar_arr_size = lock_arr_size = INITIAL_ARR_SIZE;

  return true;

cleanup:
  free(locks);

fail:
  return false;
}

void Release_Impl(UserContext* uc) {
  int lock_id = uc->regs[0];

  if (!IS_LOCK_ID(lock_id)) {
    uc->regs[0] = ERROR;
    return;
  }

  pcb_t* curr_proc = get_running_proc();

  bool res = release(GET_ID(lock_id), curr_proc, false);

  if (res) {
    uc->regs[0] = SUCCESS;
  }
  else {
    uc->regs[0] = ERROR;
  }
}

void Acquire_Impl(UserContext* uc) {
  int lock_id = uc->regs[0];

  if (!IS_LOCK_ID(lock_id)) {
    uc->regs[0] = ERROR;
    return;
  }

  pcb_t* curr_proc = get_running_proc();

  int res = acquire(GET_ID(lock_id), curr_proc, false);

  if (res == ERROR) {
    uc->regs[0] = ERROR;
  }
  else if (res == ACQUIRED) {
    uc->regs[0] = SUCCESS;
  }
  else {
    uc->regs[0] = SUCCESS;
    pcb_t* next_proc = get_next_process();

    FullContextSwitch(curr_proc, next_proc);
  }
}

void LockInit_Impl(UserContext *uc) {
  int* lock_idp = (int*) uc->regs[0];

  if (lock_idp == NULL) {
    uc->regs[0] = ERROR;
    return;
  };

  int lock_id = new_lock();

  if (lock_id == ERROR) {
    uc->regs[0] = ERROR;
  }
  else {
    uc->regs[0] = SUCCESS;
    (*lock_idp) = CREATE_ID(LOCK_FLAG, lock_id);
  }
}

void CvarInit_Impl(UserContext *uc) {
  int* cvar_idp = (int*) uc->regs[0];

  if (cvar_idp == NULL) {
    uc->regs[0] = ERROR;
    return;
  };

  int cvar_id = new_cvar();

  if (cvar_id == ERROR) {
    uc->regs[0] = ERROR;
  }
  else {
    uc->regs[0] = SUCCESS;
    (*cvar_idp) = CREATE_ID(CVAR_FLAG, cvar_id);
  }
}

void CvarWait_Impl(UserContext *uc) {
  int cvar_id = uc->regs[0];
  int lock_id = uc->regs[1];

  if (!IS_CVAR_ID(cvar_id) || !IS_LOCK_ID(lock_id)) {
    uc->regs[0] = ERROR;
    return;
  }

  pcb_t* curr_proc = get_running_proc();

  bool res = wait_cvar(GET_ID(cvar_id), GET_ID(lock_id), curr_proc);

  if (res) {
    uc->regs[0] = SUCCESS;
    pcb_t* next_proc = get_next_process();
    FullContextSwitch(curr_proc, next_proc);
  }
  else {
    uc->regs[0] = ERROR;
  }
}

void CvarSignal_Impl(UserContext *uc) {
  int cvar_id = uc->regs[0];

  if (!IS_CVAR_ID(cvar_id)) {
    uc->regs[0] = ERROR;
    return;
  }

  bool res = signal_cvar(GET_ID(cvar_id));

  uc->regs[0] = (res) ? SUCCESS : ERROR;
}

void CvarBroadcast_Impl(UserContext *uc) {
  int cvar_id = uc->regs[0];

  if (!IS_CVAR_ID(cvar_id)) {
    uc->regs[0] = ERROR;
    return;
  }

  bool res = broadcast_cvar(GET_ID(cvar_id));

  uc->regs[0] = (res) ? SUCCESS : ERROR;
}
void Reclaim_Impl(UserContext *uc) {

  int id = uc->regs[0];
  bool res = false;

  if (IS_LOCK_ID(id)) {
    res = delete_lock(GET_ID(id));
  }
  else if (IS_CVAR_ID(id)) {
    res = delete_cvar(GET_ID(id));
  }

  uc->regs[0] = (res) ? SUCCESS : ERROR;
}

static int new_cvar(void) {
  if (open_cvar_idx >= cvar_arr_size) {
    int new_size = cvar_arr_size * 2;
    cvar_t* temp = calloc(new_size, sizeof(cvar_t));

    if (temp == NULL) return -1;

    memcpy(temp, cvars, open_cvar_idx * sizeof(cvar_t));
    free(cvars);
    cvars = temp;
    cvar_arr_size = new_size;
  }
  
  int cvar_id = open_cvar_idx;

  memset(cvars + cvar_id, 0x00, sizeof(cvar_t));
  cvars[cvar_id].in_use = true;
  
  while (open_cvar_idx < cvar_arr_size && cvars[open_cvar_idx].in_use) open_cvar_idx++;

  return cvar_id;
}

static bool delete_cvar(int cvar_id) {
  if (cvar_id < 0 || cvar_id >= cvar_arr_size) return false;
  if (!cvars[cvar_id].in_use) return false;
  if (cvars[cvar_id].first != NULL) return false;

  cvars[cvar_id].in_use = false;

  if (cvar_id < open_cvar_idx) {
    open_cvar_idx = cvar_id;
  }

  for (int i = (cvar_arr_size / 2); i < cvar_arr_size; i++) {
    if (cvars[i].in_use) return true;
  }

  if (open_cvar_idx < cvar_arr_size / 2) {
    int new_size = cvar_arr_size / 2;
    cvar_t* temp = calloc(new_size, sizeof(cvar_t));

    if (temp == NULL) return true;

    memcpy(temp, cvars, new_size * sizeof(cvar_t));
    free(cvars);
    cvars = temp;
    cvar_arr_size = new_size;
  }

  return true;
}


static int new_lock(void) {
  if (open_lock_idx >= lock_arr_size) {
    int new_size = 2 * lock_arr_size;
    lock_t* temp = calloc(new_size, sizeof(lock_t));

    if (temp == NULL) return ERROR;

    memcpy(temp, locks, open_lock_idx * sizeof(lock_t));
    free(locks);
    locks = temp;
    lock_arr_size = new_size;
  }

  int lock_id = open_lock_idx;

  memset(locks + lock_id, 0x00, sizeof(lock_t));
  locks[lock_id].in_use = true;
  
  while (open_lock_idx < lock_arr_size && locks[open_lock_idx].in_use) open_lock_idx++;

  return lock_id;
}

static bool delete_lock(int lock_id) {
  if (lock_id < 0 || lock_id >= lock_arr_size) return false;
  if (!locks[lock_id].in_use) return false;
  if (locks[lock_id].needed_by > 0) return false;

  locks[lock_id].in_use = false;

  if (lock_id < open_lock_idx) {
    open_lock_idx = lock_id;
  }

  for (int i = (lock_arr_size / 2); i < lock_arr_size; i++) {
    if (locks[i].in_use) return true;
  }

  if (open_lock_idx < lock_arr_size / 2) {
    int new_size = lock_arr_size / 2;
    lock_t* temp = calloc(new_size, sizeof(lock_t));

    if (temp == NULL) return true;

    memcpy(temp, locks, (new_size) * sizeof(lock_t));
    free(locks);
    locks = temp;

    lock_arr_size = new_size;
  }

  return true;
}

static bool wait_cvar(int cvar_id, int lock_id, pcb_t* proc) {
  if (cvar_id < 0 || cvar_id >= cvar_arr_size) return false;
  if (!cvars[cvar_id].in_use) return false;

  if (!release(lock_id, proc, true)) return false;

  proc->cvar_lock_id = lock_id;
  insert(&(cvars[cvar_id].first), &(cvars[cvar_id].last), proc);
  return true;
}

static bool signal_cvar(int cvar_id) {
  if (cvar_id < 0 || cvar_id >= cvar_arr_size) return false;
  if (!cvars[cvar_id].in_use) return false;

  pcb_t* next_proc = pop(&(cvars[cvar_id].first), &(cvars[cvar_id].last));
  
  if (next_proc != NULL) {
    if (acquire(next_proc->cvar_lock_id, next_proc, true) == ACQUIRED) schedule_process(next_proc);
  };

  return true;
}

static bool broadcast_cvar(int cvar_id) {
  if (cvar_id < 0 || cvar_id >= cvar_arr_size) return false;
  if (!cvars[cvar_id].in_use) return false;

  pcb_t* next_proc;

  while ((next_proc = pop(&(cvars[cvar_id].first), &(cvars[cvar_id].last))) != NULL) {
    if (acquire(next_proc->cvar_lock_id, next_proc, true) == ACQUIRED) schedule_process(next_proc);
  }

  return true;
}

static int acquire(int lock_id, pcb_t* proc, bool cvar_use) {
  if (lock_id < 0 || lock_id >= lock_arr_size) return ERROR;
  if (!locks[lock_id].in_use) return ERROR;

  if (!locks[lock_id].acquired) {
    locks[lock_id].acquired = true;
    locks[lock_id].pid = proc->pid;
    
    if (!cvar_use) locks[lock_id].needed_by++;

    return ACQUIRED;
  }

  if (locks[lock_id].pid == proc->pid)
    return ERROR;
  
  
  insert(&(locks[lock_id].first), &(locks[lock_id].last), proc);

  if (!cvar_use) locks[lock_id].needed_by++;

  return NEED;
}

static bool release(int lock_id, pcb_t* proc, bool cvar_use) {
  if (lock_id < 0 || lock_id >= lock_arr_size) return false;
  if (!locks[lock_id].in_use) return false;
  if (!locks[lock_id].acquired || locks[lock_id].pid != proc->pid) return false;

  pcb_t* next = pop(&(locks[lock_id].first), &(locks[lock_id].last));

  if (next == NULL) {
    locks[lock_id].acquired = false;
  }
  else {
    locks[lock_id].acquired = true;
    locks[lock_id].pid = next->pid;
    schedule_process(next);
  }

  if (!cvar_use) locks[lock_id].needed_by--;

  return true;
}

static void insert(pcb_t** first, pcb_t** last, pcb_t* new_pcb) {
  new_pcb->next = NULL;

  if ((*first) == NULL) {
    (*first) = (*last) = new_pcb;
  } else {
    (*last)->next = new_pcb;
    (*last) = new_pcb;
  }
}

static pcb_t* pop(pcb_t** first, pcb_t** last) {
  if ((*first) == NULL) return NULL;

  pcb_t* ret_val = (*first);
  (*first) = ret_val->next;
  ret_val->next = NULL;

  if ((*first) == NULL) (*last) = NULL;

  return ret_val;
}
