#include "ylib.h"
#include "yuser.h"


int main(int argc, char* argv[]) {
  if (argc != 4) {
    Exit(-1);
  }
  int lock_id = atoi(argv[1]);
  int cvar_id = atoi(argv[2]);
  int chosen = atoi(argv[3]);

  if (chosen) {
    Delay(3);
    Acquire(lock_id);

    CvarBroadcast(cvar_id);
  }
  else {
    Acquire(lock_id);
    CvarWait(cvar_id, lock_id);
  }
  Release(lock_id);

  Exit(0);
}
