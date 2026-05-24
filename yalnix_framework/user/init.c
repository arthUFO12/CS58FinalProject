
#include "ylib.h"
#include "yuser.h"

/*
 * Initial userland program for Yalnix.
 * Demonstrates basic heap allocation, syscalls, and delay behavior.
 */
int main(int argc, char* argv[]) {
  int* var;
  var = (int*) malloc(sizeof(int));
  *var = 0;

  TracePrintf(0, "Pid: %d\n", GetPid());

  Brk((void*) (0x100000 + 10 * 0x2000));

  while (1) {
    (*var)++;
    TracePrintf(0, "I'm in userland!!!\n var=%d\n", *var);
    Delay(1);
  }
}
