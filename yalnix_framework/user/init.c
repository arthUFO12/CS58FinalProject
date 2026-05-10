
#include "ylib.h"


int main(int argc, char* argv[]) {
  while (1) {
    TracePrintf(0, "I'm in userland!!!\n");
    Pause();
  }
}