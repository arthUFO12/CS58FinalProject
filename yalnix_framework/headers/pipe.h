#pragma once

#include "bool.h"
#include "hardware.h"

bool init_pipes(void);
bool pipe_reclaim(int pipe_id);

void PipeInit_Impl(UserContext *uc);
void PipeRead_Impl(UserContext *uc);
void PipeWrite_Impl(UserContext *uc);
