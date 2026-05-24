
#include "ylib.h"
#include "yuser.h"


int main(int argc, char* argv[]) {
  int res;
  char* argvec[2] = {NULL, NULL};

  for (int i = 0; i < 4; i++) {
    if ((res = Fork()) == 0) {

      argvec[0] = malloc(20 * sizeof(char));
      sprintf(argvec[0], "child %d", i);
      Exec("user/test", argvec);

      TracePrintf(0, "Exec Failure. Exiting\n");
      Exit(0);	
    } else if (res == -1) {
      TracePrintf(0, "My fork failed\n");
    }
    else {
      TracePrintf(0, "Spawned child with pid %d\n", res);
    }
  }

  int status;
  for (int i = 0; i < 4; i++) {
    Wait(&status);
    TracePrintf(0, "Proc finished with exit code %d\n", status);
  }


  TracePrintf(0, "All children finished!!\n");
  while (1) {
    TracePrintf(0, "I'm in userland!!!\n");
    Pause();
  }
}
