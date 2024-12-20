#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "include/utils.h"
#include "include/config.h"
#include "include/udp_socket.h"
#include "include/git450protocol.h"

using std::string;
using std::unordered_map;
using std::vector;

int PORT = config::SERVER_D_PORT;
string SERVER_M_HOST = config::SERVER_IP;
int SERVER_M_PORT = config::SERVER_M_UDP_PORT;

class ServerD {
public:
    UDPServerSocket *server;
    unordered_map<string, vector<string> > deployed;

    ServerD() {
        server = new UDPServerSocket(PORT);
        std::cout << "Server D is up and running using UDP on port " << PORT << std::endl;
        deployed = utils::loadFileRecord("./data/deployed.txt");
    }

    ~ServerD() {
        std::cerr << "DEBUG: Server D is shutting down. Port " << PORT << " released." << std::endl;
        delete server;
    }

    void writeDeployedFile() {
        std::ofstream file("./data/deployed.txt");
        if (!file) {
            std::cerr << "Error opening file for writing: " << "./data/deployed.txt" << std::endl;
            return;
        }
        file << "UserName FileName\n";  // Add header to the file
        for (const auto &entry: deployed) {
            for (const string &filename: entry.second) {
                file << entry.first << " " << filename << std::endl;
            }
        }
    }

    void deployFiles(const string &username, const vector<string> &user_files) {
        deployed[username] = user_files;  // Overwrite the file records
    }

    void run() {
        while (true) {
            string data = server->receive();
            Git450Message request = protocol::parseMessage(data);
            vector<string> tokens = utils::split(request.payload);
            string deploy_username = tokens[0];
//            debug(deploy_username);
            vector<string> user_files(tokens.begin() + 1, tokens.end());
//            debug(utils::join(user_files));
            if (request.operation == "deploy") {
                std::cout << "Server D has received a deploy request from the main server." << std::endl;
                deployFiles(deploy_username, user_files);
                writeDeployedFile();
                std::cout << "Server D has deployed the user " << deploy_username
                          << "’s repository." << std::endl;
                string payload = utils::join(user_files);
                Git450Message response = {request.username, "deploy_result", payload};
                server->send(response.toString(), SERVER_M_HOST, SERVER_M_PORT);
                std::cout << "Server D has finished sending the response to the main server." << std::endl;
            } else {
                std::cerr << "\033[1;31mInvalid request" << request.toString() << "\033[0m" << std::endl;
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    ServerD serverD;
    serverD.run();
    return 0;
}