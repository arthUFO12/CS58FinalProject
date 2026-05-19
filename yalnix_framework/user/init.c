
#include "ylib.h"
#include "yuser.h"

int main(int argc, char* argv[]) {
  int* var;
  var = (int*) malloc(sizeof(int));
  *var = 0;

  TracePrintf(0, "Pid: %d\n", GetPid());
  while (1) {
    (*var)++;
    
    TracePrintf(0, "I'm in userland!!!\n var=%d\n", *var);
    Delay(1);
  }
}
