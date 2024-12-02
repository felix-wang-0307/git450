#include <iostream>
#include <algorithm>
#include "include/tcp_socket.h"
#include "include/udp_socket.h"
#include "include/utils.h"
#include "include/config.h"
#include "include/logger.h"
#include "include/git450protocol.h"

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

    string handleClientAuth(const string &data) const {
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

    string handleLookup(const Git450Message &request) const {
        std::cout << "The main server has received a lookup request from " << request.username
                  << " to lookup " << request.payload
                  << "’s repository using TCP over port " << SERVER_M_TCP_PORT << std::endl;
        // Forward the request to Server R
        if (!udp_client->send(request.toString(), SERVER_IP, SERVER_R_PORT)) {
            std::cerr << "Failed to send data to Server R" << std::endl;
            return "";
        }
        std::cout << "The main server has sent the lookup request to server R." << std::endl;
        string result = udp_client->receive();
        std::cout << "The main server has received the response from server R using UDP over port "
                  << SERVER_M_UDP_PORT << std::endl;
        return result;
    }

    Git450Message handlePush(const Git450Message &request) const {
        std::cout << "The main server has received a push request from " << request.username
                  << ", using TCP over port " << SERVER_M_TCP_PORT << std::endl;
        // Forward the request to Server R
        if (!udp_client->send(request.toString(), SERVER_IP, SERVER_R_PORT)) {
            std::cerr << "Failed to send data to Server R" << std::endl;
            return {};
        }
        std::cout << "The main server has sent the push request to server R using UDP over port "
                  << SERVER_M_UDP_PORT << std::endl;
        // Return the response from Server R
        string result = udp_client->receive();
        Git450Message response = protocol::parseMessage(result);
        if (response.payload == "already_exist") {
            std::cout << "The main server has received the response from server R using UDP over "
                      << SERVER_M_UDP_PORT << ", asking for overwrite confirmation" << std::endl;
        } else {
            std::cout << "The main server has received the response from server R using UDP over "
                      << SERVER_M_UDP_PORT << std::endl;
        }
        return response;
    }

    string handlePushOverwrite(const Git450Message &request) const {
        std::cout << "The main server has received the overwrite confirmation response from " << request.username
                  << ", using TCP over port " << SERVER_M_TCP_PORT << std::endl;
        // Forward the request to Server R
        if (!udp_client->send(request.toString(), SERVER_IP, SERVER_R_PORT)) {
            std::cerr << "Failed to send data to Server R" << std::endl;
            return "";
        }
        std::cout << "The main server has sent the overwrite confirmation response to server R." << std::endl;
        // Return the response from Server R
        string result = udp_client->receive();
        return result;
    }

    string handleRemove(const Git450Message &request) const {
        std::cout << "The main server has received a remove request from " << request.username
                  << ", using TCP over port " << SERVER_M_TCP_PORT << std::endl;
        // Forward the request to Server R
        if (!udp_client->send(request.toString(), SERVER_IP, SERVER_R_PORT)) {
            std::cerr << "Failed to send data to Server R" << std::endl;
            return "";
        }
        std::cout << "The main server has received confirmation of the remove request done by the server R."
                  << std::endl;
        // Return the response from Server R
        string result = udp_client->receive();
        return result;
    }

    string handleDeploy(const Git450Message &request) const {
        std::cout << "The main server has received a deploy request from " << request.username
                  << ", using TCP over port " << SERVER_M_TCP_PORT << std::endl;
        // Forward the request to Server R
        if (!udp_client->send(request.toString(), SERVER_IP, SERVER_R_PORT)) {
            std::cerr << "Failed to send data to Server R" << std::endl;
            return "";
        }
        std::cout << "The main server has sent the deploy request to server R using UDP over port "
                  << SERVER_M_UDP_PORT << std::endl;
        // Return the response from Server R
        string result = udp_client->receive();
        return result;
    }

    void run() {
        while (true) {
            // 1. receive the request from the client
            TCPSocket *client = server->accept();
            if (client == nullptr) {
                std::cerr << "Failed to accept client" << std::endl;
                continue;
            }
            string data = client->receive();
            if (data.empty()) {
                std::cerr << "Failed to receive data from the client" << std::endl;
                delete client;
                continue;
            }
            auto request = protocol::parseMessage(data);
            // 2. handle the request
            Logger *logger = new Logger();
            if (request.operation == "auth") {
                string auth_result = handleClientAuth(data);
                client->send(auth_result);
                std::cout << "The main server has sent the response from server A to client using TCP over port "
                          << SERVER_M_TCP_PORT << std::endl;
            } else if (request.operation == "lookup") {
                logger->appendLog(request);
                string result = handleLookup(request);
                client->send(result);
                std::cout << "The main server has sent the response to the client." << std::endl;
            } else if (request.operation == "push") {
                logger->appendLog(request);
                Git450Message response = handlePush(request);
                if (response.payload == "already_exist") {
                    // Ask the client for overwrite confirmation
                    client->send(response.toString());
                    std::cout << "The main server has sent the overwrite confirmation request to the client."
                              << std::endl;
                    // Receive the overwrite confirmation
                    string overwrite_data = client->receive();
                    Git450Message overwrite_request = protocol::parseMessage(overwrite_data);
                    string overwrite_result = handlePushOverwrite(overwrite_request);
                    client->send(overwrite_result);
                } else {
                    // Forward the response to the client
                    client->send(response.toString());
                    std::cout << "The main server has sent the response to the client." << std::endl;
                }
            } else if (request.operation == "remove") {
                logger->appendLog(request);
                string result = handleRemove(request);
                client->send(result);
                std::cout << "The main server has sent the response to the client." << std::endl;
            } else if (request.operation == "deploy") {
                // 1. Make a lookup request to Server R
                logger->appendLog(request);
                Git450Message lookup_request = {request.username, "lookup", request.username};
                if (!udp_client->send(lookup_request.toString(), SERVER_IP, SERVER_R_PORT)) {
                    std::cerr << "Failed to send data to Server D" << std::endl;
                    continue;
                }
                std::cout << "The main server has sent the lookup request to server R." << std::endl;
                // 2. Get the files data from Server R
                string result = udp_client->receive();
                Git450Message lookup_response = protocol::parseMessage(result);
                if (lookup_response.operation != "lookup_result") {
                    std::cerr << "Error: Invalid operation from Server R" << std::endl;
                    continue;
                }
                std::cout << "The main server received the lookup response from server R." << std::endl;
                vector<string> tokens = utils::split(lookup_response.payload);
                // The first token is the status, the rest are the files
                vector<string> files_to_deploy(tokens.begin() + 1, tokens.end());
                // 3. Make a deploy request to Server D
                // Concatenate the username and the files data
                // format: "username file1 file2 file3 ..."
                string deploy_data = request.username + " " + utils::join(files_to_deploy);
                Git450Message deploy_request = {request.username, "deploy", deploy_data};
                if (!udp_client->send(deploy_request.toString(), SERVER_IP, SERVER_D_PORT)) {
                    utils::printError("Failed to send data to Server D");
                    continue;
                }
                // 4. Forward the response to the client
                std::cout << "The main server has sent the deploy request to server D." << std::endl;
                result = udp_client->receive();
                std::cout << "The user " << request.username
                          << "’s repository has been deployed at server D." << std::endl;
                client->send(result);
            } else if (request.operation == "log") {
                string log = logger->getLogString(request.username);
                Git450Message response = {request.username, "log_result", log};
                client->send(response.toString());
                logger->appendLog(request);  // Log the "log" operation AFTERWARD
            }
            logger->writeFile();
            delete client;
            delete logger;
            std::cout << std::endl;  // Add a new line for better readability
        }
    }
};


int main() {
    ServerM serverM;
    serverM.run();
    return 0;
}