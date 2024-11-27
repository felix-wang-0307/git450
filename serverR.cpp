#include <iostream>
#include <vector>
#include <unordered_map>
#include "lib/utils.h"
#include "lib/config.h"
#include "lib/udp_socket.h"

using std::string;
using std::unordered_map;
using std::vector;

int PORT = config::SERVER_R_PORT;
string SERVER_M_HOST = config::SERVER_IP;
int SERVER_M_PORT = config::SERVER_M_UDP_PORT;

class ServerR {
public:
    UDPServerSocket *server;
    unordered_map<string, vector<string> > repo;

    ServerR() {
        server = new UDPServerSocket(PORT);
        std::cout << "Server R is up and running using UDP on port " << PORT << std::endl;
        loadRepo();  // Load the filenames from the file
    }

    ~ServerR() {
        std::cout << "DEBUG: Server R is shutting down. Port " << PORT << " released." << std::endl;
        delete server;
    }

    void loadRepo(const string &filename = "./data/filenames.txt") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }
        string line;
        std::getline(file, line);  // Skip the header
        while (std::getline(file, line)) {
            auto data = utils::split(line);
            string username = data[0];
            string filename = data[1];
            if (repo.find(username) != repo.end()) {
                repo[username].push_back(filename);
            } else {
                repo[username] = {filename};
            }
        }
    }

    void run() {
        while (true) {
            string data = server->receive();
            if (utils::getOperation(data) != "LIST") {
                std::cerr << "Invalid operation" << std::endl;
                continue;
            }
            auto listData = utils::split(data);
            const string& username = listData[1];
            std::cout << "Server R received list request from " << username << std::endl;
            vector<string> files = getFiles(username);
            string response = utils::join(files);
            if (!server->send(response, SERVER_M_HOST, SERVER_M_PORT)) {
                std::cerr << "Failed to send data to the client" << std::endl;
            }
        }
    }

    vector<string> getFiles(const string &username) {
        if (repo.find(username) != repo.end()) {
            return repo[username];
        }
        return {};
    }
};

int main() {

    return 0;
}