#pragma once
#include "hardware.h"

/* Syscall handler declarations called from trap dispatch. */
void KernelGetPid(UserContext *uc);
void KernelBrk(UserContext *uc);
void KernelDelay(UserContext *uc);
void KernelWait(UserContext *uc);
void KernelExit(UserContext *uc);
void KernelExec(UserContext *uc);
void KernelFork(UserContext *uc);
void KernelTtyRead(UserContext *uc);
void KernelTtyWrite(UserContext *uc);
