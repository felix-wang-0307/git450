
#ifndef GIT450_UDP_SOCKET_H
#define GIT450_UDP_SOCKET_H

class UDPSocket {
public:
    UDPSocket();
    ~UDPSocket();
    void bind(int port);
    void send(const char* data, int len, const char* ip, int port);
    int receive(char* data, int len, char* ip, int* port);
};

#endif //GIT450_UDP_SOCKET_H
