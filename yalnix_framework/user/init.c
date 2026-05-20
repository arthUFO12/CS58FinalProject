
#include "ylib.h"
#include "yuser.h"


int main(int argc, char* argv[]) {
  int res;

  for (int i = 0; i < 4; i++) {
    if ((res = Fork()) == 0) {
      for (int i = 0; i < 3; i++) {
        TracePrintf(0, "Hello from process %d!!! on wait %d out of 3\n", GetPid(), i + 1);
        Delay(3);
      }
      Exit(0);
    } else if (res == -1) {
      TracePrintf(0, "My fork failed\n");
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
