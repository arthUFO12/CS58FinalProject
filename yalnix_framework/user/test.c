#include "yuser.h"
#include "ylib.h"

/*
 * Simple user program that prints a greeting, sleeps, and exits.
 */
int main(int argc, char *argv[]) {
  TracePrintf(0, "Hello from %s\n", argv[0]);
  Delay(3);
  TracePrintf(0, "Exiting from %s\n", argv[0]);
  Exit(0);
}
