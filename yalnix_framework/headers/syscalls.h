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

void KernelLockInit(UserContext *uc);
void KernelAcquire(UserContext *uc);

void KernelRelease(UserContext *uc);
void KernelCvarInit(UserContext *uc);
void KernelCvarSignal(UserContext *uc);
void KernelCvarBroadcast(UserContext *uc);
void KernelCvarWait(UserContext *uc);
void KernelReclaim(UserContext *uc);
void KernelSemUp(UserContext *uc);
void KernelSemDown(UserContext *uc);
void KernelSemInit(UserContext *uc);

void KernelPipeInit(UserContext *uc);
void KernelPipeRead(UserContext *uc);
void KernelPipeWrite(UserContext *uc);
