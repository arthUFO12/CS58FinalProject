#include "hardware.h"
#include "ylib.h"
#include "yuser.h"

#define ECHO_BUF_SIZE 256
#define BIG_WRITE_LEN 2600

static void banner(int tty, const char *tag) {
  char msg[80];
  int n = sprintf(msg, "\n==== %s (tty %d) ====\n", tag, tty);
  TtyWrite(tty, msg, n);
}

static void test_basic_echo(void) {
  banner(TTY_CONSOLE, "Basic echo");

  char *prompt = "Type a line and press Enter: ";
  TtyWrite(TTY_CONSOLE, prompt, strlen(prompt));

  char buf[ECHO_BUF_SIZE];
  int n = TtyRead(TTY_CONSOLE, buf, ECHO_BUF_SIZE);
  TracePrintf(0, "echo: TtyRead returned %d\n", n);

  char hdr[64];
  int hn = sprintf(hdr, "You typed %d bytes: ", n);
  TtyWrite(TTY_CONSOLE, hdr, hn);
  if (n > 0)
    TtyWrite(TTY_CONSOLE, buf, n);
}

static void test_partial_read(void) {
  banner(TTY_CONSOLE, "Partial read");

  char *prompt = "Type a long line (>=10 chars) and press Enter: ";
  TtyWrite(TTY_CONSOLE, prompt, strlen(prompt));

  /* First read: only ask for 5 bytes of the typed line. */
  char small[5];
  int n1 = TtyRead(TTY_CONSOLE, small, 5);
  TracePrintf(0, "echo: first partial read returned %d\n", n1);

  char hdr1[80];
  int h1 = sprintf(hdr1, "first 5 bytes (%d): ", n1);
  TtyWrite(TTY_CONSOLE, hdr1, h1);
  if (n1 > 0)
    TtyWrite(TTY_CONSOLE, small, n1);
  TtyWrite(TTY_CONSOLE, "\n", 1);

  /* Second read: should be served from the buffered rx_line remainder
   * without blocking, returning the leftover bytes of the same line. */
  char rest[ECHO_BUF_SIZE];
  int n2 = TtyRead(TTY_CONSOLE, rest, ECHO_BUF_SIZE);
  TracePrintf(0, "echo: second drain read returned %d\n", n2);

  char hdr2[80];
  int h2 = sprintf(hdr2, "remainder (%d): ", n2);
  TtyWrite(TTY_CONSOLE, hdr2, h2);
  if (n2 > 0)
    TtyWrite(TTY_CONSOLE, rest, n2);
}

static void test_large_write(void) {
  banner(TTY_CONSOLE, "Large write > TERMINAL_MAX_LINE");

  char *big = malloc(BIG_WRITE_LEN);
  if (big == NULL) {
    TracePrintf(0, "echo: malloc failed for big buffer\n");
    return;
  }

  /* Fill with a repeating ABC pattern; mark every 80th column. */
  for (int i = 0; i < BIG_WRITE_LEN; i++) {
    if (i > 0 && i % 80 == 0)
      big[i] = '\n';
    else
      big[i] = 'A' + (i % 26);
  }
  big[BIG_WRITE_LEN - 1] = '\n';

  int wrote = TtyWrite(TTY_CONSOLE, big, BIG_WRITE_LEN);
  TracePrintf(0, "echo: large TtyWrite returned %d (expected %d)\n", wrote, BIG_WRITE_LEN);

  char tail[80];
  int tn = sprintf(tail, "[large write done, %d bytes]\n", wrote);
  TtyWrite(TTY_CONSOLE, tail, tn);

  free(big);
}

static void test_multi_terminal(void) {
  banner(TTY_CONSOLE, "Multi-terminal output");

  char *m0 = "hello on console (tty 0)\n";
  char *m1 = "hello on tty 1\n";
  char *m2 = "hello on tty 2\n";
  char *m3 = "hello on tty 3\n";

  TtyWrite(TTY_CONSOLE, m0, strlen(m0));
  TtyWrite(TTY_1, m1, strlen(m1));
  TtyWrite(TTY_2, m2, strlen(m2));
  TtyWrite(TTY_3, m3, strlen(m3));
}

static void child_writer(int tag) {
  char line[64];
  for (int i = 0; i < 5; i++) {
    int n = sprintf(line, "[child %d] iteration %d: the quick brown fox\n", tag, i);
    TtyWrite(TTY_CONSOLE, line, n);
    Delay(1);
  }
  Exit(tag);
}

static void test_concurrent_writers(void) {
  banner(TTY_CONSOLE, "Concurrent writers on tty 0");

  for (int i = 0; i < 2; i++) {
    int pid = Fork();
    if (pid == 0) {
      child_writer(i + 1);
      /* not reached */
    } else if (pid < 0) {
      TracePrintf(0, "echo: fork failed\n");
      return;
    } else {
      TracePrintf(0, "echo: spawned writer child pid=%d\n", pid);
    }
  }

  /* Parent also writes to interleave with both children. */
  for (int i = 0; i < 5; i++) {
    char line[64];
    int n = sprintf(line, "[parent  ] iteration %d: lazy dog\n", i);
    TtyWrite(TTY_CONSOLE, line, n);
    Delay(1);
  }

  int status;
  for (int i = 0; i < 2; i++) {
    int who = Wait(&status);
    TracePrintf(0, "echo: child %d exited with %d\n", who, status);
  }

  char *done = "[concurrent writers done]\n";
  TtyWrite(TTY_CONSOLE, done, strlen(done));
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  TracePrintf(0, "Starting echo test suite\n");

  char *hello = "echo test suite starting\n";
  TtyWrite(TTY_CONSOLE, hello, strlen(hello));

  test_multi_terminal();
  test_large_write();
  test_concurrent_writers();
  test_basic_echo();
  test_partial_read();

  char *bye = "\nAll echo tests complete. Looping on echo prompt.\n";
  TtyWrite(TTY_CONSOLE, bye, strlen(bye));

  /* Final interactive loop: classic echo. */
  char buf[ECHO_BUF_SIZE];
  for (;;) {
    char *prompt = "echo> ";
    TtyWrite(TTY_CONSOLE, prompt, strlen(prompt));
    int n = TtyRead(TTY_CONSOLE, buf, ECHO_BUF_SIZE);
    if (n <= 0)
      continue;
    TtyWrite(TTY_CONSOLE, buf, n);
  }
}
