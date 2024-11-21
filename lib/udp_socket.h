//
// Created by Waterdog on 2024/11/17.
//

#ifndef GIT450_UDP_SOCKET_H
#define GIT450_UDP_SOCKET_H

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

class UDPSocket {
public:
    UDPSocket() : socket_fd(-1) {}

    virtual ~UDPSocket() {
        if (socket_fd != -1) {
            close(socket_fd);
        }
    }

    bool create() {
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        return socket_fd != -1;
    }

    bool bind(int port) {
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        return ::bind(socket_fd, (sockaddr*)&addr, sizeof(addr)) != -1;
    }

    bool send(const std::string& data, const std::string& host, int port) {
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

        int size = ::sendto(socket_fd, data.c_str(), data.size(), 0, (sockaddr*)&addr, sizeof(addr));
        return size != -1;
    }

    std::string receive(size_t max_size = BUFFER_SIZE) {
        char *buf = new char[max_size];
        sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        ssize_t bytes_received = ::recvfrom(socket_fd, buf, max_size, 0, (sockaddr*)&addr, &addr_len);
        if (bytes_received == -1) {
            delete[] buf;
            return "";
        }
        std::string data(buf, bytes_received);
        delete[] buf;
        return data;
    }

    void closeSocket() {
        if (socket_fd != -1) {
            close(socket_fd);
            socket_fd = -1;
        }
    }
protected:
    int socket_fd;
};

class UDPServerSocket : public UDPSocket {
public:
    explicit UDPServerSocket(int port = DEFAULT_PORT) {
        if (!create()) {
            std::cerr << "Failed to create socket" << std::endl;
        }
        if (!bind(port)) {
            std::cerr << "Failed to bind socket to port " << port << std::endl;
        }
    }
};

class UDPClientSocket : public UDPSocket {
public:
    UDPClientSocket() {
        if (!create()) {
            std::cerr << "Failed to create socket" << std::endl;
        }
    }
};

#endif //GIT450_UDP_SOCKET_H
