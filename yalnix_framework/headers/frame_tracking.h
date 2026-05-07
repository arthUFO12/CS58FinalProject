
#include "bool.h"

bool acquire_frame(int vpn);
bool initialize_frame_tracking(int pmem_size);
int find_frame(void);
bool free_frame(int frame_num);

//
