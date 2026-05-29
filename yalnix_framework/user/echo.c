
#include "ylib.h"
#include "yuser.h"

int main(int argc, char *argv[]) {
  TracePrintf(0, "Starting echo...");

  char *echomsg = "Enter message to echo:\n";
  char buf[100];

  for (;;) {
    TtyWrite(0, echomsg, strlen(echomsg));
    TtyRead(0, &buf, 100);
    TtyWrite(0, &buf, 100);
  }
}
