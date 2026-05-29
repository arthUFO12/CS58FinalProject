#pragma once
#include "hardware.h"

/*
 * Terminal I/O subsystem.
 * Manages per-terminal transmit ownership, write waiter FIFOs,
 * received line buffers, and blocked readers.
 */

/* Initialize per-terminal TTY state. Call once at kernel startup. */
void init_tty(void);

/* Syscall entry points invoked from trap_kernel_handler. */
void kernel_tty_read(UserContext *uc);
void kernel_tty_write(UserContext *uc);

/* Device-trap completion entry points invoked from interrupt vector. */
void tty_transmit_done(int tty_id);
void tty_receive_done(int tty_id);
