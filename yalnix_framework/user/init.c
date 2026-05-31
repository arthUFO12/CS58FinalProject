
#include "ylib.h"
#include "yuser.h"

/*
 * Initial userland program for Yalnix.
 * This program forks several children and waits for each to exit.
 */
int main(int argc, char *argv[]) {
  int res;
  char *argvec[3] = {NULL, NULL, NULL};
  int lock_id;

  LockInit(&lock_id);

  TracePrintf(0, "Lock Id: %#x\n", lock_id);

  int res2 = Acquire(lock_id);


  /* Spawn a few child processes, each executing user/test. */
  for (int i = 0; i < 4; i++) {
    if ((res = Fork()) == 0) {
      argvec[0] = malloc(40 * sizeof(char));
      argvec[1] = malloc(40 * sizeof(char));

      
      sprintf(argvec[0], "child %d", i);
      sprintf(argvec[1], "%d", lock_id);
      Exec("user/test", argvec);

      TracePrintf(0, "Exec Failure. Exiting\n");
      Exit(0);
    } else if (res == -1) {
      TracePrintf(0, "My fork failed\n");
    } else {
      TracePrintf(0, "Spawned child with pid %d\n", res);
    }
  }

  Release(lock_id);

  /* Wait for all children to finish before entering an idle loop. */
  int status;
  for (int i = 0; i < 4; i++) {
    Wait(&status);
    TracePrintf(0, "Proc finished with exit code %d\n", status);
  }

  while (1) {
    TracePrintf(0, "I'm in userland!!!\n");
    Pause();
  }
}
