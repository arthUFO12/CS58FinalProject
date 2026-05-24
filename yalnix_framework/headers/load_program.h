
#pragma once
#include "pcb.h"

/*
 * LoadProgram replaces the current process image with a new Yalnix
 * executable and sets up the user stacks and arguments for that process.
 */
int LoadProgram(char *name, char *args[], pcb_t *proc);

