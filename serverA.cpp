#include <iostream>
#include "lib/utils.h"
#include "lib/udp_socket.h"

int PORT = std::stoi(config["server_a_port"]);
std::string SERVER_M_HOST = config["server_ip"];
int SERVER_M_PORT = std::stoi(config["server_m_port"]);

class ServerA {
public:
    UDPSocket* server;
    ServerA() {
        bootUp();
    }
    ~ServerA() {
        delete server;
    }
    void bootUp() {
        server = new UDPSocket();
        std::cout << "Server A is up and running using UDP on port " << PORT << std::endl;
    }
    void run() {
        while (true) {
            std::string data = server->receive();
            std::cout << "Server A received: " << data << std::endl;
            if (data.empty()) {
                break;
            }
            server->send(data, SERVER_M_HOST, SERVER_M_PORT);
        }
    }
};

int main() {
    return 0;
}