#include "yuser.h"
#include "ylib.h"


int main(int argc, char *argv[]) {
  if (argc != 2) {
    Exit(-1);
  }

  int num_procs = atoi(argv[1]);
  char one[] = "1";
  char zero[] = "0";
  char* child_argv[] = {"user/test_helper", NULL, NULL, one, NULL};

  int lock_id;
  int cvar_id;
  LockInit(&lock_id);
  CvarInit(&cvar_id);

  child_argv[1] = calloc(100, sizeof(char));
  child_argv[2] = calloc(100, sizeof(char));
  sprintf(child_argv[1], "%d", lock_id);
  sprintf(child_argv[2], "%d", cvar_id);

  int res;
  for (int i = 0; i < num_procs; i++) {
    if (i > 0) child_argv[3] = zero;
    if ((res = Fork()) == 0) {
      Exec(child_argv[0], child_argv);
      Exit(-1);
    } else if (res == -1) {
      Exit(-1);
    } else {
      TracePrintf(0, "Spawned child with pid %d\n", res);
    }
  }

  for (int i = 0; i < num_procs; i++) Wait(NULL);

  TracePrintf(0, "All children finished. Reclaiming lock\n");
  Reclaim(lock_id);

  Exit(0);
}
