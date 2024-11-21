#include <iostream>
#include <thread>
#include "serverM.h"
#include "lib/tcp_socket.h"
#include "lib/udp_socket.h"
#include "lib/utils.h"

class ServerM {
public:
    TCPServerSocket* server;
    UDPClientSocket* udpClient;
    ServerM() {
        bootUp();
    }
    ~ServerM() {
        delete server;
    }
    void bootUp() {
        server = new TCPServerSocket(25012);
        std::cout << "Server M is up and running using TCP on port " << TCP_PORT << std::endl;
        std::cout << "Server M is up and running using UDP on port " << UDP_PORT << std::endl;
    }
    void handleClientAuth() {

    }
    void run() {
        while (true) {
            TCPSocket* client = server->accept();
            if (client == nullptr) {
                std::cerr << "Failed to accept client" << std::endl;
                continue;
            }
            std::string data = client->receive();
            std::cout << "Server M received: " << data << std::endl;
            if (data.empty()) {
                break;
            }
            std::string operation = utils::getOperation(data);
            if(operation == "AUTH") {
                handleClientAuth();
            }
            client->send("Server M received the data");
        }
    }
};



int main() {
    ServerM serverM;
    serverM.run();
    return 0;
}