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

class ServerR {
public:
    UDPServerSocket *server;
    unordered_map<string, vector<string> > repo;

    ServerR() {
        server = new UDPServerSocket(PORT);
        std::cout << "Server R is up and running using UDP on port " << PORT << std::endl;
        repo = utils::loadFileRecord("./data/filenames.txt");  // Load the filenames from the file
    }

    ~ServerR() {
        std::cerr << "DEBUG: Server R is shutting down. Port " << PORT << " released." << std::endl;
        delete server;
    }

    vector<string> getFiles(const string &username) {
        if (repo.find(username) != repo.end()) {
            return repo[username];
        }
        return {};
    }

    // Add a file to the repository
    // Return: true if the file is added successfully, false if the file already exists
    bool addFile(const string &username, const string &filename) {
        if (repo.find(username) != repo.end()) {
            // User already exists
            vector<string> user_files = getFiles(username);
            for (auto file: user_files) {debug(file)}
            if (std::find(user_files.begin(), user_files.end(), filename) != user_files.end()) {
                // File already exists: return false and do not add the file
                return false;
            }
            repo[username].push_back(filename);
            return true;
        } else {
            // User does not exist
            repo[username] = {filename};
            return true;
        }
    }

    bool removeFile(const string &username, const string &filename) {
        if (repo.find(username) != repo.end()) {
            // User exists
            vector<string> &files = repo[username];
            auto it = std::find(files.begin(), files.end(), filename);
            if (it != files.end()) {
                // File exists
                files.erase(it);
                return true;
            }
            // File does not exist
            return false;
        }
        // User does not exist
        return false;
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

    bool overwriteFile(const string &username, const string &filename) {
        //TODO: implement the real overwrite logic in the next milestone
        return addFile(username, filename);
    }

    void run() {
        while (true) {
            string data = server->receive();
            Git450Message request = protocol::parseMessage(data);
            if (request.operation == "push") {
                std::cout << "Server R has received a push request for " << request.payload
                          << " from " << request.username << std::endl;
                bool result = addFile(request.username, request.payload);
                writeRepo();
                string payload = result ? "ok" : "already_exist";
                Git450Message response = {request.username, "push_result", payload};
                server->send(response.toString(), SERVER_M_HOST, SERVER_M_PORT);
            } else if (request.operation == "push_overwrite") {
                std::cout << "Server R has received an overwrite request for " << request.payload
                          << " from " << request.username << std::endl;
                // The payload format: "filename Y/N"
                vector<string> tokens = utils::split(request.payload);
                string &filename = tokens[0];
                bool is_overwrite = tokens[1] == "Y";
                string payload = filename + " ";
                if (is_overwrite) {
                    overwriteFile(request.username, filename);
                    writeRepo();
                    payload += "ok";
                } else {
                    payload += "abandoned";
                }
                Git450Message response = {request.username, "push_overwrite_result", payload};
                server->send(response.toString(), SERVER_M_HOST, SERVER_M_PORT);
            } else if (request.operation == "remove") {
                std::cout << "Server R has received a remove request for " << request.payload
                          << " from " << request.username << std::endl;
                bool result = removeFile(request.username, request.payload);
                writeRepo();
                string payload = result ? "ok" : "not_exist";
                Git450Message response = {request.username, "remove_result", "ok"};
                server->send(response.toString(), SERVER_M_HOST, SERVER_M_PORT);
            } else if (request.operation == "lookup") {
                std::cout << "Server R has received a lookup request from the main server." << std::endl;
                vector<string> files = getFiles(request.payload);
                string payload = utils::join(files, ' ');
                Git450Message response = {request.username, "lookup_result", payload};
                server->send(response.toString(), SERVER_M_HOST, SERVER_M_PORT);
                std::cout << "Server R has finished sending the response to the main server." << std::endl;
            } else {
                std::cerr << "\033[1;31mInvalid request: " << request.toString() << "\033[0m" << std::endl;
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    ServerR serverR;
    serverR.run();
    return 0;
}