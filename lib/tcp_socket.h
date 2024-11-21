#ifndef GIT450_TCP_SOCKET_H
#define GIT450_TCP_SOCKET_H

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

enum SocketStatus {
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
        std::cout << "Sent: " << data << std::endl;
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
        std::cout << "Received: " << data << std::endl;
        delete[] buf;
        return data;
    }

    void closeSocket() {
        if (socket_fd != -1) {
            close(socket_fd);
            socket_fd = -1;
        }
    }

    SocketStatus getStatus() {
        return status;
    }

    int getSocketFd() {
        return socket_fd;
    }

    void setSocketFd(int fd) {
        socket_fd = fd;
    }

protected:
    SocketStatus status;
    int socket_fd;
};

class TCPServerSocket : public TCPSocket {
public:
    int socket_name;

    explicit TCPServerSocket(int port = DEFAULT_PORT) {
        if (!create()) {
            std::cerr << "Failed to create socket" << std::endl;
            status = SocketStatus::ERROR;
            return;
        }
        if (!bind(port)) {
            std::cerr << "Failed to bind socket to port " << port << std::endl;
            status = SocketStatus::ERROR;
            return;
        }
        if (!listen()) {
            std::cerr << "Failed to listen on socket" << std::endl;
            status = SocketStatus::ERROR;
            return;
        }
        status = SocketStatus::LISTENING;
        std::cout << "Server is listening on port " << port << std::endl;
    }

    bool bind(int port) {
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;  // Bind to all interfaces
        addr.sin_port = htons(port);

        int opt = 1;
        // Allow the socket to reuse the address
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
            return false;
        }

        // Attempt to bind
        if (::bind(socket_fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
            std::cerr << "Bind failed: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }


    bool listen(int backlog = 5) {
        if (::listen(socket_fd, backlog) == -1) {
            std::cerr << "Listen failed: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

    TCPSocket* accept() {
        int client_sock = ::accept(socket_fd, nullptr, nullptr);
        if (client_sock == -1) {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            return nullptr;
        }
        TCPSocket* newSocket = new TCPSocket();
        newSocket->setSocketFd(client_sock);
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

#endif //GIT450_TCP_SOCKET_H