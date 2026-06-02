#include "yuser.h"
#include "ylib.h"


int main(int argc, char *argv[]) {
  if (argc != 3) {
    Exit(-1);
  }

  int num_procs = atoi(argv[1]);
  int sem_val = atoi(argv[2]);

  char* child_argv[] = {"user/test_helper2", NULL, NULL};

  int sem_id;
  SemInit(&sem_id, sem_val);

  child_argv[1] = calloc(100, sizeof(char));
  sprintf(child_argv[1], "%d", sem_id);

  int res;
  for (int i = 0; i < num_procs; i++) {
    if ((res = Fork()) == 0) {
      Exec(child_argv[0], child_argv);
      Exit(-1);
    } else if (res == -1) {
      Exit(-1);
    } else {
      TracePrintf(0, "Spawned child with pid %d\n", res);
    }
  }

  for (int i = 0; i < num_procs; i++) Wait(NULL);

  TracePrintf(0, "All children finished. Reclaiming sem\n");
  Reclaim(sem_id);

  Exit(0);

}
