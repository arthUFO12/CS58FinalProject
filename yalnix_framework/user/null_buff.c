#include "ylib.h"
#include "yuser.h"


int main() {
  TtyRead(2, NULL, 2000);
  TtyRead(2, 0x1ffffff, 20000);


  return 0;
}
