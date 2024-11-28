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
        std::cerr << "DEBUG: Server R is shutting down. Port " << PORT << " released." << std::endl;
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
            if (utils::getOperation(data) != "push" && utils::getOperation(data) != "remove" && utils::getOperation(data) != "lookup") {
                std::cerr << "Invalid operation" << std::endl;
                continue;
            }
            string operation = utils::getOperation(data);
            string username = utils::getUsername(data);
            string payload = utils::getPayload(data);
            if (operation == "push") {
                addFile(username, payload);
                std::cout << "Server R has received a push request for " << payload << " from " << username << std::endl;
            } else if (operation == "remove") {
                removeFile(username, payload);
                std::cout << "Server R has received a remove request for " << payload << " from " << username << std::endl;
            } else if (operation == "lookup") {
                std::cout << "Server R has received a lookup request from the main server." << std::endl;
                vector<string> files = getFiles(username);
                string response = "lookup_result " + utils::join(files);
                server->send(response, SERVER_M_HOST, SERVER_M_PORT);
                std::cout << "Server R has finished sending the response to the main server." << std::endl;
            }
        }
    }

    vector<string> getFiles(const string &username) {
        if (repo.find(username) != repo.end()) {
            return repo[username];
        }
        return {};
    }

    void addFile(const string &username, const string &filename) {
        if (repo.find(username) != repo.end()) {
            repo[username].push_back(filename);
        } else {
            repo[username] = {filename};
        }
    }

    void removeFile(const string &username, const string &filename) {
        if (repo.find(username) != repo.end()) {
            vector<string> &files = repo[username];
            auto it = std::find(files.begin(), files.end(), filename);
            if (it != files.end()) {
                files.erase(it);
            }
        }
    }

    void writeRepo(const string &filename = "./data/filenames.txt") {
        std::ofstream file(filename);
        if (!file) {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
            return;
        }
        file << "UserName FileName\n";  // Add header to the file
        for (const auto &entry: repo) {
            for (const string &filename: entry.second) {
                file << entry.first << " " << filename << std::endl;
            }
        }
    }
};

int main() {

    return 0;
}