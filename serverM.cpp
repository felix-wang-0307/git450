#include <iostream>
#include "lib/tcp_socket.h"
#include "lib/udp_socket.h"
#include "lib/utils.h"
#include "lib/config.h"

using namespace config;
using std::string;

class ServerM {
public:
    TCPServerSocket *server;
    UDPClientSocket *udp_client;

    ServerM() {
        bootUp();
    }

    ~ServerM() {
        std::cout << "DEBUG: Server M is shutting down. Port " << SERVER_M_TCP_PORT << " and " << SERVER_M_UDP_PORT
                  << " released." << std::endl;
        delete server;
        delete udp_client;
    }

    void bootUp() {
        server = new TCPServerSocket(SERVER_M_TCP_PORT);
        udp_client = new UDPClientSocket(SERVER_M_UDP_PORT);
        std::cout << "Server M is up and running using TCP on port " << SERVER_M_TCP_PORT << std::endl;
        std::cout << "Server M is up and running using UDP on port " << SERVER_M_UDP_PORT << std::endl;
    }

    string handleClientAuth(const string &data) {
        auto auth_data = utils::split(data);
        const string &username = auth_data[1];
        const string &password = auth_data[2];
        std::cout << "Server M has received username " << username
                  << " and password " << utils::toAstrix(password)
                  << std::endl;
        if (!udp_client->send(data, SERVER_IP, SERVER_A_PORT)) {
            std::cerr << "Failed to send data to Server A" << std::endl;
            return "";
        }
        std::cout << "Server M has sent authentication request to Server A" << std::endl;
        string auth_result = udp_client->receive();
        if (utils::getOperation(auth_result) != "AUTH_RESULT") {
            std::cerr << "Invalid operation" << std::endl;
            return "";
        }
        std::cout << "The main server has received the response from server A using UDP over port "
                  << SERVER_M_UDP_PORT << std::endl;
        return auth_result;
    }

    void run() {
        while (true) {
            TCPSocket *client = server->accept();
            if (client == nullptr) {
                std::cerr << "Failed to accept client" << std::endl;
                continue;
            }
            string data = client->receive();
            if (data.empty()) {
                break;
            }
            string operation = utils::toUpper(utils::getOperation(data));
            if (operation == "AUTH") {
                string auth_result = handleClientAuth(data);
                client->send(auth_result);
                std::cout << "The main server has sent the response from server A to client using TCP over port "
                          << SERVER_M_TCP_PORT << std::endl;
            } else if (operation == "LOOKUP" || operation == "PUSH" || operation == "REMOVE") {
                // Forward the request to Server R
                if (!udp_client->send(data, SERVER_IP, SERVER_R_PORT)) {
                    std::cerr << "Failed to send data to Server R" << std::endl;
                    continue;
                }
                std::cout << "The main server has sent the lookup request to server R using UDP over port "
                          << SERVER_M_UDP_PORT << std::endl;
            } else if (operation == "DEPLOY") {
                // Forward the request to Server D
                if (!udp_client->send(data, SERVER_IP, SERVER_D_PORT)) {
                    std::cerr << "Failed to send data to Server D" << std::endl;
                    continue;
                }
                std::cout << "The main server has sent the deploy request to server D using UDP over port "
                          << SERVER_M_UDP_PORT << std::endl;
            } else if (operation == "LOG") {

            }

            delete client;
        }
    }
};


int main() {
    ServerM serverM;
    serverM.run();
    return 0;
}