# git450
## Introduction
The socket programming project of EE450, 2024 Fall, University of Southern California. 

This project implements a Git-like project version control system using C/C++ and socket libraries.

## Files
```yaml
- README.md: This file.
- lib
  - udp_socket.h: provides a minimal UDP socket class for simplifying the socket programming.
  - tcp_socket.h: provides a minimal TCP socket class for simplifying the socket programming.
  - utils.h: provides some utility functions like "split".
```

## Runtime Environment
- OS: Ubuntu 20.04
- Compiler: g++ 9.3.0
- C++ Standard: C++17