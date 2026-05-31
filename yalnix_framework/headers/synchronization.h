

#include "hardware.h"
#include "bool.h"

bool init_sync(void);
void LockInit_Impl(UserContext *uc);


void Release_Impl(UserContext* uc);


void Acquire_Impl(UserContext* uc);

void CvarInit_Impl(UserContext *uc);

void CvarWait_Impl(UserContext *uc);

void CvarSignal_Impl(UserContext *uc);

void CvarBroadcast_Impl(UserContext *uc);

void Reclaim_Impl(UserContext *uc);



