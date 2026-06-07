#include "yuser.h"



int main() {
  *((volatile int *)0x00);
  Exit(0);
}
