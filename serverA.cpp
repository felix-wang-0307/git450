#include <iostream>
#include <unordered_map>
#include <string>
#include "lib/config.h"
#include "lib/udp_socket.h"
#include "lib/encryptor.h"
#include "lib/utils.h"
#include "lib/git450protocol.h"

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
        std::cerr << "DEBUG: Server A is shutting down. Port " << PORT << " released." << std::endl;
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

    bool findIfMemberExists(const string &username) {
        return members.find(username) != members.end();
    }

    void run() {
        while (true) {
            string data = server->receive();
            Git450Message request = protocol::parseMessage(data);
            if (request.operation != "auth") {
                std::cerr << "Invalid operation" << std::endl;
                continue;
            }
            const string& username = request.username;
            const string& password = request.payload;
            std::cout << "Server A received username " << username
                      << " and password " << utils::toAstrix(password)
                      << std::endl;
            ClientType type = authenticate(username, password);
            if (static_cast<int>(type) >= 1) {
                std::cout << "Member " << username
                          << " has been authorized" << std::endl;
            } else {
                std::cout << "The username " << username
                          << " or password " << utils::toAstrix(password)
                          << " is incorrect" << std::endl;
            }
            Git450Message auth_message = {username, "auth_result", ClientTypeToString.at(type)};
            server->send(auth_message.toString(), SERVER_M_HOST, SERVER_M_PORT);
            std::cout << std::endl;
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