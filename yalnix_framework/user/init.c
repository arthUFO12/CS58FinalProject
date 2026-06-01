
#include "ylib.h"
#include "yuser.h"

/*
 * Initial userland program for Yalnix.
 * This program forks several children and waits for each to exit.
 */
int main(int argc, char *argv[]) {
  char** child_argv = argv + 1;
  char* child_name = child_argv[0];

  int res;
  if ((res = Fork()) == 0) {
    Exec(child_name, child_argv);

    Exit(-1);
  }
  else if (res != -1) {
    TracePrintf(0, "Started the %s program!\n", child_name);
  }
  else {
    TracePrintf(0, "Couldn't start %d program. Exiting init process\n", child_name);
    Exit(-1);
  }

  while (1) {
    TracePrintf(0, "I'm in userland!!!\n");
    Wait(NULL);
  }
}
