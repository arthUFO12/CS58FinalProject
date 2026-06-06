#include "pipe.h"

#include "identifiers.h"
#include "interrupt.h"
#include "pcb.h"
#include "scheduler.h"
#include "yalnix.h"
#include "ylib.h"

#ifndef ERROR
#define ERROR -1
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#define INITIAL_PIPE_COUNT 10

typedef struct {
  bool in_use;
  char *buf;
  int cap;
  int head;
  int len;
  pcb_t *read_head;
  pcb_t *read_tail;
} pipe_t;

static pipe_t *pipes;
static int pipes_size;
static int open_pipe_idx;

static int new_pipe(void);
static pipe_t *get_pipe(int pipe_id);
static bool ensure_capacity(pipe_t *pipe, int needed);
static void append_bytes(pipe_t *pipe, char *buf, int len);
static int read_bytes(pipe_t *pipe, char *buf, int len);
static void readq_push(pipe_t *pipe, pcb_t *proc);
static pcb_t *readq_pop(pipe_t *pipe);
static void wake_readers(pipe_t *pipe);

bool init_pipes(void) {
  pipes = calloc(INITIAL_PIPE_COUNT, sizeof(pipe_t));
  if (pipes == NULL)
    return false;

  pipes_size = INITIAL_PIPE_COUNT;
  open_pipe_idx = 0;
  return true;
}

void PipeInit_Impl(UserContext *uc) {
  int *pipe_idp = (int *)uc->regs[0];
  if (pipe_idp == NULL) {
    uc->regs[0] = ERROR;
    return;
  }

  int pipe_id = new_pipe();
  if (pipe_id == ERROR) {
    uc->regs[0] = ERROR;
    return;
  }

  *pipe_idp = CREATE_ID(PIPE_FLAG, pipe_id);
  uc->regs[0] = SUCCESS;
}

void PipeRead_Impl(UserContext *uc) {
  int pipe_id = (int)uc->regs[0];
  char *buf = (char *)uc->regs[1];
  int len = (int)uc->regs[2];

  if (!IS_PIPE_ID(pipe_id) || buf == NULL || len < 0) {
    uc->regs[0] = ERROR;
    return;
  }
  if (len == 0) {
    uc->regs[0] = 0;
    return;
  }

  pipe_id = GET_ID(pipe_id);
  pipe_t *pipe = get_pipe(pipe_id);
  if (pipe == NULL) {
    uc->regs[0] = ERROR;
    return;
  }

  while (pipe->len == 0) {
    pcb_t *curr_proc = get_running_proc();
    readq_push(pipe, curr_proc);
    curr_proc->state = WAITING;

    pcb_t *next_proc = get_next_process();
    FullContextSwitch(curr_proc, next_proc);

    pipe = get_pipe(pipe_id);
    if (pipe == NULL) {
      uc->regs[0] = ERROR;
      return;
    }
  }

  uc->regs[0] = read_bytes(pipe, buf, len);
}

void PipeWrite_Impl(UserContext *uc) {
  int pipe_id = (int)uc->regs[0];
  char *buf = (char *)uc->regs[1];
  int len = (int)uc->regs[2];

  if (!IS_PIPE_ID(pipe_id) || buf == NULL || len < 0) {
    uc->regs[0] = ERROR;
    return;
  }
  if (len == 0) {
    uc->regs[0] = 0;
    return;
  }

  pipe_t *pipe = get_pipe(GET_ID(pipe_id));
  if (pipe == NULL || !ensure_capacity(pipe, pipe->len + len)) {
    uc->regs[0] = ERROR;
    return;
  }

  append_bytes(pipe, buf, len);
  wake_readers(pipe);
  uc->regs[0] = len;
}

bool pipe_reclaim(int pipe_id) {
  pipe_t *pipe = get_pipe(pipe_id);
  if (pipe == NULL || pipe->read_head != NULL)
    return false;

  free(pipe->buf);
  memset(pipe, 0x00, sizeof(pipe_t));
  if (pipe_id < open_pipe_idx)
    open_pipe_idx = pipe_id;

  return true;
}

static int new_pipe(void) {
  if (open_pipe_idx >= pipes_size) {
    int new_size = pipes_size * 2;
    pipe_t *temp = calloc(new_size, sizeof(pipe_t));
    if (temp == NULL)
      return ERROR;

    memcpy(temp, pipes, pipes_size * sizeof(pipe_t));
    free(pipes);
    pipes = temp;
    pipes_size = new_size;
  }

  int pipe_id = open_pipe_idx;
  memset(pipes + pipe_id, 0x00, sizeof(pipe_t));

  pipes[pipe_id].buf = malloc(PIPE_BUFFER_LEN);
  if (pipes[pipe_id].buf == NULL)
    return ERROR;

  pipes[pipe_id].cap = PIPE_BUFFER_LEN;
  pipes[pipe_id].in_use = true;

  while (open_pipe_idx < pipes_size && pipes[open_pipe_idx].in_use)
    open_pipe_idx++;

  return pipe_id;
}

static pipe_t *get_pipe(int pipe_id) {
  if (pipe_id < 0 || pipe_id >= pipes_size || !pipes[pipe_id].in_use)
    return NULL;

  return pipes + pipe_id;
}

static bool ensure_capacity(pipe_t *pipe, int needed) {
  if (needed <= pipe->cap)
    return true;

  int new_cap = pipe->cap;
  while (new_cap < needed)
    new_cap *= 2;

  char *new_buf = malloc(new_cap);
  if (new_buf == NULL)
    return false;

  for (int i = 0; i < pipe->len; i++)
    new_buf[i] = pipe->buf[(pipe->head + i) % pipe->cap];

  free(pipe->buf);
  pipe->buf = new_buf;
  pipe->cap = new_cap;
  pipe->head = 0;
  return true;
}

static void append_bytes(pipe_t *pipe, char *buf, int len) {
  for (int i = 0; i < len; i++) {
    pipe->buf[(pipe->head + pipe->len) % pipe->cap] = buf[i];
    pipe->len++;
  }
}

static int read_bytes(pipe_t *pipe, char *buf, int len) {
  int bytes = (pipe->len < len) ? pipe->len : len;

  for (int i = 0; i < bytes; i++) {
    buf[i] = pipe->buf[pipe->head];
    pipe->head = (pipe->head + 1) % pipe->cap;
  }

  pipe->len -= bytes;
  if (pipe->len == 0)
    pipe->head = 0;

  return bytes;
}

static void readq_push(pipe_t *pipe, pcb_t *proc) {
  proc->next = NULL;
  if (pipe->read_tail == NULL) {
    pipe->read_head = proc;
  } else {
    pipe->read_tail->next = proc;
  }
  pipe->read_tail = proc;
}

static pcb_t *readq_pop(pipe_t *pipe) {
  pcb_t *proc = pipe->read_head;
  if (proc == NULL)
    return NULL;

  pipe->read_head = proc->next;
  if (pipe->read_head == NULL)
    pipe->read_tail = NULL;

  proc->next = NULL;
  return proc;
}

static void wake_readers(pipe_t *pipe) {
  pcb_t *proc;
  while ((proc = readq_pop(pipe)) != NULL)
    schedule_process(proc);
}
