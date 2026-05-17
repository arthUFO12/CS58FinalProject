#ifndef SYSCALL_H
#define SYSCALL_H

#include "hardware.h"

void KernelFork(UserContext *uc);
void KernelExec(UserContext *uc);
void KernelExit(UserContext *uc);
void KernelWait(UserContext *uc);
void KernelGetPid(UserContext *uc);
void KernelBrk(UserContext *uc);
void KernelDelay(UserContext *uc);
void KernelTtyRead(UserContext *uc);
void KernelTtyWrite(UserContext *uc);

#endif
