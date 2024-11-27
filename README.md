# git450
## Introduction
The socket programming project of EE450, 2024 Fall, University of Southern California. 

This project implements a Git-like project version control system using C/C++ and socket libraries.

## Files
- README.md: This file.
- makefile: The Makefile for the project.
- lib
  - config.h: provides some configuration parameters for the project.
  - udp_socket.h: provides a minimal **UDP socket class** for simplifying the socket programming.
  - tcp_socket.h: provides a minimal **TCP socket class** for simplifying the socket programming.
  - utils.h: provides some utility functions like `split` and `join`.

## Runtime Environment
- OS: Ubuntu 20.04
    - May work on macOS or other Linux distributions, but not guaranteed.
- Compiler: g++ 9.3.0
- C++ Standard: C++11
- Make: GNU Make 4.2.1

## Implementation
### Network Topology
![img_1.png](img_1.png)
### Transport-Layer Protocol
**TCP** is used for client-server communication.

**UDP** is used for server-server communication.

### Application-Layer Protocol
The protocol is designed as follows:
```plaintext
[operation] [username] [payload]
```
- operation (client): one of the following:
  - `auth`: Authentication operation
  - `lookup <username>`: Lookup the files of a user.
- operation (server): same as above with each operation suffixed with `_result`.
- username: the username of the client.
- payload: the payload (or the data) of the operation.

Example:
1. Client sends an authentication request to ServerM over TCP
    ```plaintext
    AUTH user1 password1
    ```
2. ServerM transfers the authentication request to ServerA over UDP
    ```plaintext
    AUTH user1 password1
    ```
3. ServerA responds to ServerM over UDP
    ```plaintext
    AUTH_RESULT user1 MEMBER
    ```
4. ServerM transfers the authentication result to the client over TCP
    ```plaintext
    AUTH_RESULT user1 MEMBER
    ```
   

### TCP/UDP Socket Classes
To simplify the socket programming, I implemented two classes, `TCPSocket` and `UDPSocket`, which encapsulate the socket programming details. The prototypes of the classes are as follows:


