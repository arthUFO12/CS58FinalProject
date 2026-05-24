# Yalnix Framework Layout

## 1. src, user, and headers:
All of our code 

## 2. include 
Where all of the important definition for the framework live


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
