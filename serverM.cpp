#include <iostream>
#include <algorithm>
#include "lib/tcp_socket.h"
#include "lib/udp_socket.h"
#include "lib/utils.h"
#include "lib/config.h"
#include "lib/logger.h"
#include "lib/git450protocol.h"

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
        auto request = protocol::parseMessage(data);
        string &password = request.payload;
        std::cout << "Server M has received username " << request.username
                  << " and password " << utils::toAstrix(password)
                  << std::endl;
        if (!udp_client->send(data, SERVER_IP, SERVER_A_PORT)) {
            std::cerr << "Failed to send data to Server A" << std::endl;
            return "";
        }
        std::cout << "Server M has sent authentication request to Server A" << std::endl;
        string auth_result = udp_client->receive();
        if (protocol::getOperation(auth_result) != "auth_result") {
            std::cerr << "Error: Invalid operation from Server A" << std::endl;
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
            debug(data)
            if (data.empty()) {
                std::cerr << "Failed to receive data from the client" << std::endl;
                delete client;
                continue;
            }
            auto request = protocol::parseMessage(data);
            debug(request.toString());
            Logger *logger = new Logger();
            if (request.operation == "auth") {
                string auth_result = handleClientAuth(data);
                client->send(auth_result);
                std::cout << "The main server has sent the response from server A to client using TCP over port "
                          << SERVER_M_TCP_PORT << std::endl;
            } else if (utils::contains({"lookup", "deploy", "push"}, request.operation)) {
                // Forward the request to Server R
                logger->appendLog(request);
                if (!udp_client->send(data, SERVER_IP, SERVER_R_PORT)) {
                    std::cerr << "Failed to send data to Server R" << std::endl;
                    continue;
                }
                std::cout << "The main server has sent the lookup request to server R using UDP over port "
                          << SERVER_M_UDP_PORT << std::endl;
                // Forward the response to the client
                string result = udp_client->receive();
                client->send(result);
            } else if (request.operation == "deploy") {
                // 1. Make a lookup request to Server R
                logger->appendLog(request);
                Git450Message lookup_request = {request.username, "lookup", request.payload};
                if (!udp_client->send(lookup_request.toString(), SERVER_IP, SERVER_R_PORT)) {
                    std::cerr << "Failed to send data to Server D" << std::endl;
                    continue;
                }
                std::cout << "The main server has sent the deploy request to server R using UDP over port "
                          << SERVER_M_UDP_PORT << std::endl;
                // 2. Get the files data from Server R
                string result = udp_client->receive();
                Git450Message lookup_response = protocol::parseMessage(result);
                if (lookup_response.operation != "lookup_result") {
                    std::cerr << "Error: Invalid operation from Server R" << std::endl;
                    continue;
                }
                string deploy_files = lookup_response.payload;
                // 3. Make a deploy request to Server D
                // Concatenate the username and the files data
                string deploy_data = request.username + " " + deploy_files;
                Git450Message deploy_request = {request.username, "deploy", deploy_data};
                if (!udp_client->send(deploy_request.toString(), SERVER_IP, SERVER_D_PORT)) {
                    std::cerr << "Failed to send data to Server D" << std::endl;
                    continue;
                }
                // 4. Forward the response to the client
                std::cout << "The main server has sent the deploy request to server D using UDP over port "
                          << SERVER_M_UDP_PORT << std::endl;
                result = udp_client->receive();
                client->send(result);
            } else if (request.operation == "log") {
                logger->appendLog(request);  // Log the "log" operation
                string log = logger->getLogString(request.username);
                Git450Message response = {request.username, "log_result", log};
                client->send(response.toString());
            }
            logger->writeFile();
            delete client;
            delete logger;
        }
    }
};


int main() {
    ServerM serverM;
    serverM.run();
    return 0;
}