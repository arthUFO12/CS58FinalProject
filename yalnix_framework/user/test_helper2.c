
#include "ylib.h"
#include "yuser.h"


int main(int argc, char* argv[]) {
  if (argc != 2) {
    Exit(-1);
  }
  int sem_id = atoi(argv[1]);
 
  SemDown(sem_id);
  Delay(5);
  SemUp(sem_id);
  
  Exit(0);
}
