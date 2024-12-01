#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "lib/utils.h"
#include "lib/config.h"
#include "lib/udp_socket.h"
#include "lib/git450protocol.h"

using std::string;
using std::unordered_map;
using std::vector;

int PORT = config::SERVER_R_PORT;
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

    void deployFiles(const string &username, const vector<string>& user_files) {
        if (deployed.find(username) != deployed.end()) {
            for (const string &filename: user_files) {
                if (!utils::contains(deployed[username], filename)) {
                    deployed[username].push_back(filename);
                }
            }
        } else {
            deployed[username] = user_files;
        }
    }

    void run() {
        while (true) {
            string data = server->receive();
            Git450Message request = protocol::parseMessage(data);
            vector<string> tokens = utils::split(request.payload);
            string deploy_username = tokens[0];
            vector<string> user_files(tokens.begin() + 1, tokens.end());
            if (request.operation == "deploy") {
                std::cout << "Server D has received a deploy request from the main server." << std::endl;
                deployFiles(deploy_username, user_files);
                writeDeployedFile();
                Git450Message response = {request.username, "deploy_result", "ok"};
                server->send(response.toString(), SERVER_M_HOST, SERVER_M_PORT);
                std::cout << "Server D has finished sending the response to the main server." << std::endl;
            } else {
                std::cerr << "\033[1;31mInvalid request" << request.toString() << "\033[0m" << std::endl;
            }
        }
    }
};

int main() {
    ServerD serverD;
    serverD.run();
    return 0;
}