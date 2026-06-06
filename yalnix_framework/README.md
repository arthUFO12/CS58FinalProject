# Yalnix Framework Layout

- `src/` Project sourcecode
- `headers/` Project headers
- `user/` User test programs

# Work done in Checkpoint4
We have implemented `Fork`, `Exec`, `Wait`, and `Exit`. We have also implemented the math and illegal traps which simply exit the process.
The implementations for the sycalls are in `src/syscalls.c` and the traps are in `src/interrupt.c`.

# Checkpoint 4 Testing

The `init` program tests the `Fork`, `Exec`, and `Wait` (and by extension `Exit`) system calls before entering its infinite loop.
The `init` spawns 4 new processes using `Fork` and prints out their pids. It then calls `Wait` four consecutive times and prints 
the status codes it stores.
The child processes store their names in a buffer before passing the buffer as an argument to `Exec`. 
`Exec` then runs the `test` program which prints out the name passed to it from the command line arguments, delays for 3 clock ticks
and then exits.

The program works as intended, sucessfully spawning all 4 processes and then switching execution to the `test` program. Running 

```bash
make
./yalnix user/init
```

Will run the init program with this testing.

# Checkpoint 5 Testing

The `user/echo.c` program validates TTY behavior:
- Basic echo (read line, echo back)
- Partial read (short read then drain remainder)
- Large write > `TERMINAL_MAX_LINE` (spans lines)
- Multi‑terminal output (TTY 0‑3)
- Concurrent writers (parent + two children)
- Interactive echo loop (continuous prompt)

Build and run:

```bash
make
./yalnix -x user/echo
```

# Pipe Testing

The `user/pipe_test.c` program validates the pipe subsystem:
- PipeInit creates a pipe and returns a valid pipe_id.
- Child process blocks on `PipeRead` until the parent writes.
- Parent writes “hello” → child reads exactly 5 bytes, verifies string.
- Parent waits for child exit and checks status code.
- Partial writes/reads test buffering (write 6, read 3, then drain remaining 3).
- Final write/read of “xyz” confirms full‑cycle behavior.
- Reclaim frees the pipe; successful run prints “pipe_test PASS”.

Build and run:

```bash
make
./yalnix user/pipe_test
```
