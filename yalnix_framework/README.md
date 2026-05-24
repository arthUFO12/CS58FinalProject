# Yalnix Framework Layout

## 1. src and headers:
All of our code 

## 2. include 
Where all of the important definition for the framework live


# Testing
The `init` program tests all 3 syscalls `Brk`, `GetPid`, and `Delay`. To run the kernel with the testing file as init run:
```bash
make
./yalnix user/init
```

The program prints all messages from the `Brk` and `GetPid` syscalls as expected and also waits the desired amount of clock ticks
after each `Delay` call.
