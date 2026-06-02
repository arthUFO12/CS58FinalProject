#include "ylib.h"
#include "yuser.h"

/*
 * Initial userland program for Yalnix.
 * This program forks several children and waits for each to exit.
 */
int main(int argc, char *argv[]) {
  char* child_argv[] = {NULL, NULL};

  int res;

  for (int i = 1; i < argc; i++) {
    if ((res = Fork()) == 0) {
      child_argv[0] = argv[i];
      Exec(argv[i], child_argv);

      Exit(-1);
    }
    else if (res != -1) {
      TracePrintf(0, "Started the %s program!\n", argv[i]);
    }
    else {
      TracePrintf(0, "Couldn't start %d program.\n", argv[i]);
      Exit(-1);
    }
  }

  while (1) {
    TracePrintf(0, "I'm in userland!!!\n");
    Wait(NULL);
  }
}
