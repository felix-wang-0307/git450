#include <iostream>
#include "serverM.h"
#include "lib/tcpSocket.h"
#include "lib/udpSocket.h"
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
        server = new TCPServerSocket(TCP_PORT);
        std::cout << "Server M is up and running using UDP on port " << UDP_PORT << std::endl;
    }
    void handleClientAuth() {

    }
    void run() {
        server->listen();
        while (true) {
            TCPSocket* client = server->accept();
            std::string data = client->receive();
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