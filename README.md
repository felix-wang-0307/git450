# git450
## Introduction
The socket programming project of EE450, 2024 Fall, University of Southern California. 

This project implements a Git-like project version control system using C/C++ and socket libraries.

## Files
- `README.md`: This file.
- `makefile`: The Makefile for the project.
- lib
  - `git450protocol.h`: provides the protocol for the Git450 application.
  - `config.h`: provides some configuration parameters for the project.
  - `udp_socket.h`: provides a minimal **UDP socket class** for simplifying the socket programming.
  - `tcp_socket.h`: provides a minimal **TCP socket class** for simplifying the socket programming.
  - `utils.h`: provides some utility functions like `split` and `join`.
  - `logger.h`: provides a simple logger for `log` command.
  - `encryptor.h`: provides a encryptor for the project.

## Runtime Environment
- OS: Ubuntu 20.04
    - You can install docker according to [USC CSCI-104 Docker Tutorial](https://github.com/csci104/docker) to run the project.
    - *Might* work on macOS or other Linux distributions, but not guaranteed.
- Compiler: g++ 9.3.0
- C++ Standard: C++11
- Make: GNU Make 4.2.1

## Implementation
### Network Topology
![img_1.png](img_1.png)
### Transport-Layer Protocol
**TCP** is used for client-server communication.

**UDP** is used for server-server communication.

### Git450 Application-Layer Protocol
A Git450 protocol is designed to facilitate the communication between the client and the server. 
It runs between the client and the server, and between the servers. **NOT** in user's command lines. User commands are parsed and translated into Git450 protocol by the client, and the server processes the Git450 protocol and returns the result to the client.

The protocol is designed as follows:
```plaintext
[username] [operation] [payload]
```
- username: the username of the client.
- operation (client): one of the following:
  - `auth`: Authentication operation
    - This should **not** be in command. It should be sent automatically when a client boots up.
  - `lookup <username>`: Lookup the files of a user.
- operation (server): same as above with each operation suffixed with `_result`.
- payload: the payload (or the data) of the operation.

Example:
1. Client sends an authentication request to ServerM over TCP
    ```plaintext
    user1 auth password1
    ```
2. ServerM transfers the authentication request to ServerA over UDP
    ```plaintext
    user1 auth password1
    ```
3. ServerA responds to ServerM over UDP
    ```plaintext
    user1 auth_result MEMBER
    ```
4. ServerM transfers the authentication result to the client over TCP
    ```plaintext
    user1 auth_result MEMBER
    ```



### TCP/UDP Socket Classes
To simplify the socket programming, I implemented two classes, `TCPSocket` and `UDPSocket`, which encapsulate the socket programming details. The prototypes of the classes are as follows:

## Acknowledgement
Thanks to Professor [Ali Zahid](https://viterbi.usc.edu/directory/faculty/Zahid/Ali) and TAs for the guidance and support in EE450.

Thanks to the following resources and tools:
- [Computer Networking: A Top-Down Approach](https://www.amazon.com/Computer-Networking-Top-Down-Approach-7th/dp/0133594149)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/)
- [C++11 Standard](https://en.cppreference.com/w/cpp/11)
- [GitHub Copilot](https://copilot.github.com/)

Thanks to the verse that inspired me not to give up the gracious 15 points after endlessly tackling the fancy bugs and the beautiful Segmentation Faults:
> Isaiah 40:31 But those who hope in the Lord will renew their strength. They will soar on wings like eagles; they will run and not grow weary, they will walk and not be faint.
