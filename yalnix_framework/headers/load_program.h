
#include "pcb.h"

/*
 * Load a user program into the address space of the given process.
 * - name: path to the executable file in Yalnix format.
 * - args: argv-style argument list.
 * - proc: PCB of the process receiving the program image.
 * Returns SUCCESS or ERROR as defined by the kernel.
 */
int LoadProgram(char *name, char *args[], pcb_t* proc);

