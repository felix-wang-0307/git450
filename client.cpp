#include <iostream>
#include <map>
#include <thread>
#include "client.h"
#include "lib/tcp_socket.h"
#include "lib/utils.h"

std::map<std::string, std::string> config;

enum ClientType {
    INVALID = -1,
    GUEST,
    USER
};

class Client {
public:
    TCPClientSocket* client;
    int type;
    Client(std::string username, std::string password) {
        bootUp();
        type = authenticate(username, password);
        if (type == -1) {
            std::cerr << "Failed to authenticate" << std::endl;
            exit(1);
        }
    }
    ~Client() {
        delete client;
    }
    void bootUp() {
        client = new TCPClientSocket();
        std::string server_ip = config["server_ip"];
        int server_port = std::stoi(config["server_port"]);
        std::cout << "Client is up and running" << std::endl;
        if (!client->connect(server_ip, server_port)) {
            std::cerr << "Failed to connect to the server" << std::endl;
            exit(1);
        }
    }
    int authenticate(const std::string& username, const std::string& password) {
        // Send the username and password to the server
        std::string data = "AUTH " + username + " " + password;
        client->send(data);
        std::string response = client->receive();
        // Check the response: USER or GUEST
        if (response == "USER") {
            return USER;
        } else if (response == "GUEST") {
            return GUEST;
        } else {
            return INVALID;
        }
    }
    void run() {
        while (true) {

        }
    }
};

int main(int argc, char* argv[]) {
    // Run the client: ./client <username> <password>
    config = utils::loadConfig();
    if (argc < 3) {
        std::cerr << "Wrong number of arguments: expect 2, get " << argc << std::endl;
        std::cerr << "Usage: ./client <username> <password>" << std::endl;
        exit(1);
    }
    Client client(argv[1], argv[2]);
    client.run();
    return 0;
}

