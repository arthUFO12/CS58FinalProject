#include "yuser.h"
#include "ylib.h"

/*
 * Simple user program that prints a greeting, sleeps, and exits.
 */
int main(int argc, char *argv[]) {
  int lock_id = atoi(argv[1]);
  Acquire(lock_id);
  TracePrintf(0, "Hello from %s\n", argv[0]);
  Delay(3);
  TracePrintf(0, "Exiting from %s\n", argv[0]);
  Release(lock_id);
  Exit(0);
}
