#ifndef GIT450_TCPSOCKET_H
#define GIT450_TCPSOCKET_H

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

enum class SocketType {
    SERVER,
    CLIENT
};

enum class SocketStatus {
    ERROR = -1,
    DISCONNECTED,
    CONNECTED,
    LISTENING,
};

class TCPSocket {
public:
    TCPSocket() : socket_fd(-1), status(SocketStatus::DISCONNECTED) {}

    virtual ~TCPSocket() {
        if (socket_fd != -1) {
            close(socket_fd);
        }
    }

    bool create() {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        return socket_fd != -1;
    }

    bool send(const std::string& data) {
        int size = ::send(socket_fd, data.c_str(), data.size(), 0);
        return size != -1;
    }

    std::string receive(size_t max_size = BUFFER_SIZE) {
        char *buf = new char[max_size];
        ssize_t bytes_received = ::recv(socket_fd, buf, max_size, 0);
        if (bytes_received == -1) {
            delete[] buf;
            status = SocketStatus::ERROR;
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
    SocketStatus status;
    int socket_fd;
};

class TCPServerSocket : public TCPSocket {
public:
    explicit TCPServerSocket(int port = DEFAULT_PORT) {
        if (!create()) {
            std::cerr << "Failed to create socket" << std::endl;
            status = SocketStatus::ERROR;
        }
        if (!bind(port)) {
            std::cerr << "Failed to bind socket to port " << port << std::endl;
            status = SocketStatus::ERROR;
        }
        if (!listen()) {
            std::cerr << "Failed to listen on socket" << std::endl;
            status = SocketStatus::ERROR;
        }
    }

    bool bind(int port) {
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        return ::bind(socket_fd, (sockaddr*)&addr, sizeof(addr)) != -1;
    }

    bool listen(int backlog = 5) {
        return ::listen(socket_fd, backlog) != -1;
    }

    TCPSocket* accept() {
        int client_sock = ::accept(socket_fd, nullptr, nullptr);
        if (client_sock == -1) {
            return nullptr;
        }

        TCPSocket* newSocket = new TCPSocket();
        newSocket->socket_fd = client_sock;
        return newSocket;
    }
};

class TCPClientSocket : public TCPSocket {
public:
    TCPClientSocket() {
        if (!create()) {
            std::cerr << "Failed to create socket" << std::endl;
            status = SocketStatus::ERROR;
        }
    }

    bool connect(const std::string& host, int port) {
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

        return ::connect(socket_fd, (sockaddr*)&addr, sizeof(addr)) != -1;
    }
};

#endif //GIT450_TCPSOCKET_H