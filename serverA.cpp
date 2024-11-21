#include <iostream>
#include <unordered_map>
#include <string>
#include "lib/config.h"
#include "lib/udp_socket.h"
#include "lib/encryptor.h"
#include "lib/utils.h"

using std::string;
using std::unordered_map;

int PORT = config::SERVER_A_PORT;
string SERVER_M_HOST = config::SERVER_IP;
int SERVER_M_PORT = config::SERVER_M_UDP_PORT;

class ServerA {
public:
    UDPServerSocket *server;
    unordered_map<string, string> members;

    ServerA() {
        server = new UDPServerSocket(PORT);
        std::cout << "Server A is up and running using UDP on port " << PORT << std::endl;
        loadMembers();  // Load the members from the file
    }

    ~ServerA() {
        std::cout << "DEBUG: Server A is shutting down. Port " << PORT << " released." << std::endl;
        delete server;
    }

    void loadMembers(const string &filename = "./data/members.txt") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }
        string line;
        std::getline(file, line);  // Skip the header
        while (std::getline(file, line)) {
            auto data = utils::split(line);
            members[data[0]] = data[1];
        }
    }

    void run() {
        while (true) {
            string data = server->receive();
            if (utils::getOperation(data) != "AUTH") {
                std::cerr << "Invalid operation" << std::endl;
                continue;
            }
            auto authData = utils::split(data);
            const string& username = authData[1];
            const string& password = authData[2];
            std::cout << "Server A received username " << username
                      << " and password " << utils::toAstrix(password)
                      << std::endl;
            ClientType type = authenticate(username, password);
            debug(ClientTypeToString.at(type));
            if (static_cast<int>(type) >= 0) {
                std::cout << "Member " << username
                          << " has been authorized";
            } else {
                std::cout << "The username " << username
                          << " or password " << utils::toAstrix(password)
                          << " is incorrect";
            }
            string auth_message = "AUTH_RESULT " + ClientTypeToString.at(type);
            debug(auth_message);
            server->send(auth_message, SERVER_M_HOST, SERVER_M_PORT);
        }
    }

    ClientType authenticate(const string &username, const string &password) {
        if (username == "guest" && password == "guest") {
            return ClientType::GUEST;
        }
        if (members.find(username) == members.end()) {
            return ClientType::NOT_EXIST;
        }
        string true_password = members[username];
        if (true_password == Encryptor::encrypt(password)) {
            return ClientType::MEMBER;
        }
        return ClientType::INVALID;  // invalid password
    }
};

int main() {
    ServerA serverA;
    serverA.run();
    return 0;
}