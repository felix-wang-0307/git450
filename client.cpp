#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <algorithm>
#include "lib/tcp_socket.h"
#include "lib/utils.h"
#include "lib/config.h"
#include "lib/git450protocol.h"

using std::string;
using std::vector;

string SERVER_HOST = config::SERVER_IP;  // All servers are running on the same host 127.0.0.1
int SERVER_PORT = config::SERVER_M_TCP_PORT;  // Client will connect to Server M using TCP

const vector<string> GUEST_COMMANDS = {"lookup"};
const vector<string> MEMBER_COMMANDS = {"lookup", "push", "remove", "deploy", "log"};

class Client {
public:
    TCPClientSocket *client;
    ClientType type;
    string username;

    Client(string username, string password) {
        bootUp();
        type = authenticate(username, password);
        if (type == ClientType::GUEST) {
            this->username = "guest";
            std::cout << "You have been granted guest access." << std::endl;
        } else if (type == ClientType::MEMBER) {
            this->username = username;
            std::cout << "You have been granted member access." << std::endl;
        } else {
            // Wrong password or username not found
            std::cerr << "The credentials are incorrect. Please try again." << std::endl;
            delete client;
            exit(1);
        }
    }

    ~Client() {
        delete client;
    }

    void bootUp() {
        client = new TCPClientSocket();
        std::cout << "Client is up and running" << std::endl;
        if (!client->connect(SERVER_HOST, SERVER_PORT)) {
            std::cerr << "Failed to connect to the server" << std::endl;
            exit(1);
        }
    }

    ClientType authenticate(const string &username, const string &password) {
        // Send the username and password to the server
        Git450Message request = {username, "auth", password};
        client->send(request.toString());
        string _response = client->receive();
        Git450Message response = protocol::parseMessage(_response);
        // Check the response
        if (response.operation != "auth_result") {
            std::cerr << "Invalid response from the server" << std::endl;
            exit(1);
        }
        // Convert the payload to ClientType
        return StringToClientType.at(response.payload);
    }

    bool isOperationValid(const string &operation) const {
        if (type == ClientType::GUEST) {
            return std::find(GUEST_COMMANDS.begin(), GUEST_COMMANDS.end(), operation) != GUEST_COMMANDS.end();
        } else if (type == ClientType::MEMBER) {
            return std::find(MEMBER_COMMANDS.begin(), MEMBER_COMMANDS.end(), operation) != MEMBER_COMMANDS.end();
        }
        return false;
    }

    void run() {
        while (true) {
            if (type == ClientType::MEMBER) {
                std::cout << "\nPlease enter the command: " << std::endl
                          << "<lookup <username>>" << std::endl
                          << "<push <filename>>" << std::endl
                          << "<remove <filename>>" << std::endl
                          << "<deploy>" << std::endl
                          << "<log>" << std::endl;
            } else { // GUEST
                std::cout << "Please enter the command: " << std::endl
                          << "<lookup <username>>" << std::endl;
            }
            string command;
            std::getline(std::cin, command);
            command = utils::trim(command);  // Remove leading and trailing spaces
            // 1. Check if the command is "exit"
            if (command == "exit") {
                break;
            }
            // 2. Check if the command is valid for the client type
            vector<string> parts = utils::split(command, ' ');
            if (parts.empty()) {
                std::cerr << "Empty command." << std::endl;
                continue;
            }
            string operation = parts[0];
            if (!isOperationValid(operation)) {
                std::cerr << "Invalid operation: " << operation << std::endl;
                continue;
            }
            // 3. If the command is valid, connect to serverM, add the username and send it to serverM
            // Make sure the message complies with Git450 Protocol: <username> <operation> <payload>
            delete client;  // Make sure the previous connection is closed
            client = new TCPClientSocket();  // Create a new connection for EACH REQUEST (non-persistent)
            if (!client->connect(SERVER_HOST, SERVER_PORT)) {
                std::cerr << "ERROR: Failed to connect to the server" << std::endl;
                continue;
            }
            string data = username + " " + command;
            // 4. Show the message of sent successfully
            Git450Message request = protocol::parseMessage(data);
            if (request.operation == "lookup") {
                if (request.payload.empty()) {
                    std::cout << "Username is not specified. Will lookup " << request.username << ".\n";
                    request.payload = request.username;
                }
                std::cout << request.username << " sent a lookup request to the main server." << std::endl;
            } else if (request.operation == "push") {
                if (request.payload.empty()) {
                    std::cout << "Error: Filename is required. Please specify a filename to push." << std::endl;
                    std::cout << "—--Start a new request—--" << std::endl;
                    continue;
                }
            } else {
                // log, remove, deploy
                std::cout << request.username << " sent a "
                          << request.operation << " request to the main server." << std::endl;
            }
            client->send(request.toString());
            // 5. Receive the response from serverM and parse the data
            data = client->receive();
            Git450Message response = protocol::parseMessage(data);
            if (response.operation == "lookup_result") {
                std::cout << "The client received the response from the main server using TCP over port "
                          << SERVER_PORT << "." << std::endl;
                vector<string> files = utils::split(response.payload, ' ');
                if (files.empty()) {
                    string lookup_username = request.payload.empty() ? request.username : request.payload;
                    std::cout << lookup_username << " does not exist. Please try again." << std::endl;
                    continue;
                }
                std::cout << utils::join(files, '\n') << std::endl;
            } else if (response.operation == "push_result") {
                string file_to_push = request.payload;
                if (response.payload == "already_exist") {
                    //TODO: change the username
                    std::cout << file_to_push << " exists in "
                              << request.username << "’s repository, do you want to overwrite (Y/N)?"
                              << std::endl;
                    std::getline(std::cin, command);
                    command = utils::trim(command);
                    // Make sure the command is either "Y" or "N"
                    Git450Message push_overwrite_request = {
                            request.username,
                            "push_overwrite",
                            file_to_push + " "
                    };
                    if (command == "Y" || command == "N") {
                        push_overwrite_request.payload += command;
                    } else {
                        utils::printError("Invalid command. Will abandon the overwrite.");
                        push_overwrite_request.payload = " ABANDON";
                    }
                    // Send the overwrite request (without closing the connection)
                    client->send(push_overwrite_request.toString());
                    // Receive overwrite response (receipt)
                    data = client->receive();
                    Git450Message push_overwrite_response = protocol::parseMessage(data);
                    if (push_overwrite_response.operation == "push_overwrite_result") {
                        vector<string> tokens = utils::split(push_overwrite_response.payload);
                        const string &filename = tokens[0];
                        const string &result = tokens[1];
                        if (result == "ok") {
                            std::cout << filename << " pushed successfully." << std::endl;
                        } else {
                            std::cout << filename << " was not pushed successfully." << std::endl;
                        }
                    }
                } else {
                    std::cout << request.payload << " pushed successfully" << std::endl;
                }
            } else if (response.operation == "remove_result") {
                std::cout << "The remove request was successful." << std::endl;
            } else if (response.operation == "deploy_result") {
                vector<string> files = utils::split(response.payload, ' ');
                if (files.empty()) {
                    std::cout << "The client received the response from the main server using TCP over port "
                              << SERVER_PORT
                              << ", and no files were found for deployment." << std::endl;
                } else {
                    std::cout << "The client received the response from the main server using TCP over port "
                              << SERVER_PORT
                              << ". The following files in his/her repository have been deployed." << std::endl;
                    // print deployed files in lines
                    std::cout << utils::join(files, '\n') << std::endl;
                }
            } else if (response.operation == "log_result") {
                std::cout << response.payload << std::endl;
            } else {
                utils::printError("Invalid response from the server: " + response.toString());
            }
        }
    }

};

Client *global_client = nullptr;

void handleSigint(int signal) {
    std::cout << "\nCaught signal " << signal << ". Shutting down client..." << std::endl;
    delete global_client; // Ensures the destructor is called
    exit(0);
}


int main(int argc, char *argv[]) {
    signal(SIGINT, handleSigint);
    signal(SIGTERM, handleSigint);
    // Run the client: ./client <username> <password>
    if (argc < 3) {
        std::cerr << "Wrong number of arguments: expect 2, get " << argc - 1 << std::endl;
        std::cerr << "Usage: ./client <username> <password>" << std::endl;
        exit(1);
    }
    try {
        global_client = new Client(argv[1], argv[2]);
        global_client->run();
    } catch (const std::exception &e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        delete global_client; // Ensure cleanup on exception
        return 1;
    }
    delete global_client;  // Ensure cleanup after "exit" command
    return 0;
}

