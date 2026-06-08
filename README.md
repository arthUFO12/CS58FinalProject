# Yalnix Operating System

This repository implements a UNIX-based kernel for the DCS58 (a virtualized 32-bit x86 machine) complete with virtual memory management, round-robin process scheduling, synchronization primitives, terminal IO, and IPC using pipes. It implements most of the major Unix syscalls like `fork`, `exec`, and `wait`, and maintains isolation even under heavy memory load or user error.

Details [here](https://github.com/arthUFO12/CS58FinalProject/tree/main/yalnix_framework)

- [Headers](https://github.com/arthUFO12/CS58FinalProject/tree/main/yalnix_framework/headers)
- [Source Code](https://github.com/arthUFO12/CS58FinalProject/tree/main/yalnix_framework/src)
- [User Test Programs](https://github.com/arthUFO12/CS58FinalProject/tree/main/yalnix_framework/user)
