#include "yuser.h"
#include "ylib.h"

#define CHECK(cond, msg)                                                       \
  do {                                                                         \
    if (!(cond)) {                                                             \
      TracePrintf(0, "pipe_test FAIL: %s\n", msg);                            \
      Exit(-1);                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  int pipe_id;
  int rc;
  int status;
  char buf[32];

  TracePrintf(0, "pipe_test: start\n");

  rc = PipeInit(&pipe_id);
  CHECK(rc == 0, "PipeInit");
  TracePrintf(0, "pipe_test: pipe_id=%d\n", pipe_id);

  rc = Fork();
  CHECK(rc >= 0, "Fork reader");

  if (rc == 0) {
    TracePrintf(0, "pipe_test: child reading, should block\n");

    rc = PipeRead(pipe_id, buf, 5);
    CHECK(rc == 5, "child PipeRead count");

    buf[5] = '\0';
    TracePrintf(0, "pipe_test: child read [%s]\n", buf);
    CHECK(strcmp(buf, "hello") == 0, "child FIFO data");

    Exit(17);
  }

  Delay(3);

  rc = PipeWrite(pipe_id, "hello", 5);
  CHECK(rc == 5, "PipeWrite waking child");

  rc = Wait(&status);
  CHECK(rc == 0, "Wait child");
  CHECK(status == 17, "child exit status");

  rc = PipeWrite(pipe_id, "abcdef", 6);
  CHECK(rc == 6, "PipeWrite partial source");

  rc = PipeRead(pipe_id, buf, 3);
  CHECK(rc == 3, "partial read count");
  buf[3] = '\0';
  TracePrintf(0, "pipe_test: partial read 1 [%s]\n", buf);
  CHECK(strcmp(buf, "abc") == 0, "partial read 1 data");

  rc = PipeRead(pipe_id, buf, 10);
  CHECK(rc == 3, "partial read drain count");
  buf[3] = '\0';
  TracePrintf(0, "pipe_test: partial read 2 [%s]\n", buf);
  CHECK(strcmp(buf, "def") == 0, "partial read 2 data");

  rc = PipeWrite(pipe_id, "xyz", 3);
  CHECK(rc == 3, "PipeWrite final");

  rc = PipeRead(pipe_id, buf, 3);
  CHECK(rc == 3, "final read count");
  buf[3] = '\0';
  CHECK(strcmp(buf, "xyz") == 0, "final read data");

  rc = Reclaim(pipe_id);
  CHECK(rc == 0, "Reclaim pipe");

  TracePrintf(0, "pipe_test PASS\n");
  Exit(0);
}
