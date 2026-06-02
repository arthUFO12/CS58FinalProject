#include "hardware.h"
#include "yalnix.h"
#include "ykernel.h"
#include "ylib.h"

#include "interrupt.h"
#include "pcb.h"
#include "scheduler.h"
#include "tty.h"

typedef struct rx_line {
  char buf[TERMINAL_MAX_LINE];
  int len;
  int off;
  struct rx_line *next;
} rx_line_t;

typedef struct {
  /* Write */
  pcb_t *tx_owner;
  bool transmitting;
  int tx_last_chunk;
  pcb_t *w_head, *w_tail;

  /* Read */
  rx_line_t *rx_head, *rx_tail;
  pcb_t *r_head, *r_tail;
} tty_state_t;

static tty_state_t ttys[NUM_TERMINALS];

/* Queue helpers */

static void wq_push(tty_state_t *t, pcb_t *p) {
  p->next = NULL;
  if (t->w_tail == NULL) {
    t->w_head = p;
  } else {
    t->w_tail->next = p;
  }
  t->w_tail = p;
}

static pcb_t *wq_pop(tty_state_t *t) {
  pcb_t *p = t->w_head;
  if (p == NULL)
    return NULL;
  t->w_head = p->next;
  if (t->w_head == NULL)
    t->w_tail = NULL;
  p->next = NULL;
  return p;
}

static void rq_push(tty_state_t *t, pcb_t *p) {
  p->next = NULL;
  if (t->r_tail == NULL) {
    t->r_head = p;
  } else {
    t->r_tail->next = p;
  }
  t->r_tail = p;
}

static pcb_t *rq_pop(tty_state_t *t) {
  pcb_t *p = t->r_head;
  if (p == NULL)
    return NULL;
  t->r_head = p->next;
  if (t->r_head == NULL)
    t->r_tail = NULL;
  p->next = NULL;
  return p;
}

/* Transmit next chunk */

static void start_chunk(int tty_id) {
  tty_state_t *t = &ttys[tty_id];
  pcb_t *owner = t->tx_owner;
  int remaining = owner->io_total - owner->io_sent;

  int chunk = remaining < TERMINAL_MAX_LINE ? remaining : TERMINAL_MAX_LINE;

  t->tx_last_chunk = chunk;

  t->transmitting = true;
  TracePrintf(2, "tty %d: TtyTransmit owner=%d off=%d chunk=%d total=%d\n", tty_id, owner->pid, owner->io_sent, chunk,
              owner->io_total);
  TtyTransmit(tty_id, owner->io_kbuf + owner->io_sent, chunk);
}

/* Reader copy helper */

static int consume_into_user(tty_state_t *t, void *ubuf, int len) {
  rx_line_t *line = t->rx_head;

  if (line == NULL)
    return 0;

  int avail = line->len - line->off;
  int n = avail < len ? avail : len;
  memcpy(ubuf, line->buf + line->off, n);

  line->off += n;
  if (line->off >= line->len) {
    t->rx_head = line->next;

    if (t->rx_head == NULL)
      t->rx_tail = NULL;
    free(line);
  }

  return n;
}

void init_tty(void) { memset(ttys, 0, sizeof(ttys)); }

void kernel_tty_write(UserContext *uc) {
  int tty_id = (int)uc->regs[0];
  void *ubuf = (void *)uc->regs[1];
  int len = (int)uc->regs[2];

  if (tty_id < 0 || tty_id >= NUM_TERMINALS || len < 0 || ubuf == NULL) {
    uc->regs[0] = ERROR;
    return;
  }
  if (len == 0) {
    uc->regs[0] = 0;
    return;
  }

  pcb_t *curr = get_running_proc();
  tty_state_t *t = &ttys[tty_id];

  /* Copy from user to Region 0 buffer */
  curr->io_kbuf = malloc(len);
  if (curr->io_kbuf == NULL) {
    uc->regs[0] = ERROR;
    return;
  }
  memcpy(curr->io_kbuf, ubuf, len);
  curr->io_total = len;
  curr->io_sent = 0;
  curr->io_tty_id = tty_id;

  if (t->tx_owner == NULL) {
    t->tx_owner = curr;
    start_chunk(tty_id);
  } else {
    wq_push(t, curr);
  }

  curr->state = WAITING;
  pcb_t *next = get_next_process();
  FullContextSwitch(curr, next);

  /* Resumed after all chunks complete. */
  free(curr->io_kbuf);
  curr->io_kbuf = NULL;
  uc->regs[0] = curr->io_total;
}

void kernel_tty_read(UserContext *uc) {
  int tty_id = (int)uc->regs[0];
  void *ubuf = (void *)uc->regs[1];
  int len = (int)uc->regs[2];

  if (tty_id < 0 || tty_id >= NUM_TERMINALS || len < 0 || ubuf == NULL) {
    uc->regs[0] = ERROR;
    return;
  }
  if (len == 0) {
    uc->regs[0] = 0;
    return;
  }

  pcb_t *curr = get_running_proc();
  tty_state_t *t = &ttys[tty_id];

  if (t->rx_head != NULL) {
    uc->regs[0] = consume_into_user(t, ubuf, len);
    return;
  }

  curr->io_user_buf = ubuf;
  curr->io_len = len;
  curr->io_tty_id = tty_id;
  rq_push(t, curr);

  curr->state = WAITING;
  pcb_t *next = get_next_process();
  FullContextSwitch(curr, next);

  uc->regs[0] = consume_into_user(t, curr->io_user_buf, curr->io_len);
}

/* Traps */

void tty_transmit_done(int tty_id) {
  if (tty_id < 0 || tty_id >= NUM_TERMINALS)
    return;

  tty_state_t *t = &ttys[tty_id];
  pcb_t *owner = t->tx_owner;

  if (owner == NULL) {
    TracePrintf(0, "tty %d: transmit complete\n", tty_id);
    return;
  }

  owner->io_sent += t->tx_last_chunk;
  t->transmitting = false;
  TracePrintf(2, "tty %d: transmit_done owner=%d sent=%d/%d\n", tty_id, owner->pid, owner->io_sent, owner->io_total);

  if (owner->io_sent < owner->io_total) {
    start_chunk(tty_id);
    return;
  }

  /* Owner finished. Wake it and start next waiter*/
  t->tx_owner = NULL;
  schedule_process(owner);

  pcb_t *nxt = wq_pop(t);
  if (nxt != NULL) {
    t->tx_owner = nxt;
    start_chunk(tty_id);
  }
}

void tty_receive_done(int tty_id) {
  if (tty_id < 0 || tty_id >= NUM_TERMINALS)
    return;
  tty_state_t *t = &ttys[tty_id];

  rx_line_t *line = malloc(sizeof(rx_line_t));
  if (line == NULL) {
    TracePrintf(0, "tty %d: rx OOM, dropping input\n", tty_id);
    return;
  }
  line->len = TtyReceive(tty_id, line->buf, TERMINAL_MAX_LINE);
  line->off = 0;
  line->next = NULL;
  if (t->rx_tail) {
    t->rx_tail->next = line;
  } else {
    t->rx_head = line;
  }
  t->rx_tail = line;
  TracePrintf(2, "tty %d: receive_done len=%d\n", tty_id, line->len);

  pcb_t *r = rq_pop(t);
  if (r != NULL)
    schedule_process(r);
}
