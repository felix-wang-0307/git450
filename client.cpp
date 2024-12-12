#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <algorithm>
#include <memory>
#include "include/tcp_socket.h"
#include "include/utils.h"
#include "include/config.h"
#include "include/git450protocol.h"
#include "include/message_types.h"

using std::string;
using std::vector;

string SERVER_HOST = config::SERVER_IP;
int SERVER_PORT = config::SERVER_M_TCP_PORT;

const vector<string> GUEST_COMMANDS = {"lookup"};
const vector<string> MEMBER_COMMANDS = {"lookup", "push", "remove", "deploy", "log"};

class Client {
public:
    std::unique_ptr<TCPClientSocket> client;
    ClientType type;
    string username;

    Client(const string& username, const string& password) {
        bootUp();
        type = authenticate(username, password);
        if (type == ClientType::GUEST) {
            this->username = "Guest";
            std::cout << "You have been granted guest access." << std::endl;
        } else if (type == ClientType::MEMBER) {
            this->username = username;
            std::cout << "You have been granted member access." << std::endl;
        } else {
            std::cerr << "The credentials are incorrect. Please try again." << std::endl;
            exit(1);
        }
    }

    void bootUp() {
        client = std::make_unique<TCPClientSocket>();
        std::cout << "Client is up and running" << std::endl;
        if (!client->connect(SERVER_HOST, SERVER_PORT)) {
            std::cerr << "Failed to connect to the server" << std::endl;
            exit(1);
        }
    }

    ClientType authenticate(const string& username, const string& password) {
        if (username == "guest" && password == "guest") {
            return ClientType::GUEST;
        }
        Git450Message request = {username, "auth", password};
        client->send(request.toString());
        string response_data = client->receive();
        Git450Message response = protocol::parseMessage(response_data);

        if (response.operation != "auth_result") {
            std::cerr << "Invalid response from the server" << std::endl;
            exit(1);
        }
        return StringToClientType.at(response.payload);
    }

    bool isOperationValid(const string& operation) const {
        if (type == ClientType::GUEST) {
            return std::find(GUEST_COMMANDS.begin(), GUEST_COMMANDS.end(), operation) != GUEST_COMMANDS.end();
        } else if (type == ClientType::MEMBER) {
            return std::find(MEMBER_COMMANDS.begin(), MEMBER_COMMANDS.end(), operation) != MEMBER_COMMANDS.end();
        }
        return false;
    }

    void run() {
        while (true) {
            displayCommandPrompt();

            string command;
            std::getline(std::cin, command);
            command = utils::trim(command);

            if (command == "exit") {
                std::cout << "Bye! Shutting down the client..." << std::endl;
                break;
            }

            vector<string> parts = utils::split(command, ' ');
            if (parts.empty()) {
                std::cerr << "Empty command." << std::endl;
                continue;
            }

            string operation = parts[0];
            if (!isOperationValid(operation)) {
                utils::printError(type == ClientType::GUEST ? "Guests can only use the lookup command." : "Invalid operation: " + operation);
                continue;
            }

            // Prepare a new socket connection for each request (non-persistent)
            reconnectToServer();

            // Create request message
            std::unique_ptr<Git450Message> request = createRequest(operation, parts);

            // Send request
            client->send(request->toString());

            // Receive response
            string data = client->receive();
            std::unique_ptr<Git450Message> response = protocol::parseMessage(data);

            handleResponse(*request, *response);
        }
    }

private:
    void displayCommandPrompt() const {
        if (type == ClientType::MEMBER) {
            std::cout << "Please enter the command: " << std::endl
                      << "<lookup <username>>" << std::endl
                      << "<push <filename>>" << std::endl
                      << "<remove <filename>>" << std::endl
                      << "<deploy>" << std::endl
                      << "<log>" << std::endl;
        } else { // GUEST
            std::cout << "Please enter the command: " << std::endl
                      << "<lookup <username>>" << std::endl;
        }
    }

    void reconnectToServer() {
        client = std::make_unique<TCPClientSocket>(); // Recreate connection for each request
        if (!client->connect(SERVER_HOST, SERVER_PORT)) {
            std::cerr << "ERROR: Failed to connect to the server" << std::endl;
        }
    }

    std::unique_ptr<Git450Message> createRequest(const string& operation, const vector<string>& parts) {
        std::unique_ptr<Git450Message> request;

        if (operation == "lookup") {
            request = std::make_unique<Git450LookupRequest>(username, parts.size() > 1 ? parts[1] : "");
        } else if (operation == "push") {
            request = std::make_unique<Git450PushRequest>(username, parts.size() > 1 ? parts[1] : "");
        } else if (operation == "remove") {
            request = std::make_unique<Git450RemoveRequest>(username, parts.size() > 1 ? parts[1] : "");
        } else if (operation == "deploy") {
            request = std::make_unique<Git450DeployRequest>(username);
        } else if (operation == "log") {
            request = std::make_unique<Git450LogRequest>(username);
        }
        return request;
    }

    void handleResponse(const Git450Message& request, const Git450Message& response) {
        if (response.operation == "lookup_result") {
            std::cout << "Response from server: " << response.payload << std::endl;
        } else if (response.operation == "push_result") {
            std::cout << "Push response: " << response.payload << std::endl;
        } else if (response.operation == "remove_result") {
            std::cout << "Remove response: " << response.payload << std::endl;
        } else if (response.operation == "deploy_result") {
            std::cout << "Deploy response: " << response.payload << std::endl;
        } else if (response.operation == "log_result") {
            std::cout << "Log response: " << response.payload << std::endl;
        } else {
            utils::printError("Invalid response from the server: " + response.toString());
        }
    }
};

// Global client instance for signal handling
std::unique_ptr<Client> global_client;

void handleSigint(int signal) {
    std::cout << "\nCaught signal " << signal << ". Shutting down client..." << std::endl;
    global_client.reset(); // Automatically cleans up
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handleSigint);
    signal(SIGTERM, handleSigint);

    if (argc < 3) {
        std::cerr << "Wrong number of arguments: expect 2, get " << argc - 1 << std::endl;
        std::cerr << "Usage: ./client <username> <password>" << std::endl;
        exit(1);
    }

    try {
        global_client = std::make_unique<Client>(argv[1], argv[2]);
        global_client->run();
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        global_client.reset(); // Ensure cleanup on exception
        return 1;
    }

    global_client.reset(); // Ensure cleanup after "exit" command
    return 0;
}
