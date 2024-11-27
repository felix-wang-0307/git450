#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include "lib/tcp_socket.h"
#include "lib/utils.h"
#include "lib/config.h"

using std::string;

string SERVER_HOST = config::SERVER_IP;  // All servers are running on the same host 127.0.0.1
int SERVER_PORT = config::SERVER_M_TCP_PORT;  // Client will connect to Server M using TCP

const std::vector<string> GUEST_COMMANDS = {"lookup"};
const std::vector<string> MEMBER_COMMANDS = {"lookup", "push", "remove", "deploy", "log"};

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
        string data = "AUTH " + username + " " + password;
        client->send(data);
        string response = client->receive();
        // Check the response
        if (utils::getOperation(response) != "AUTH_RESULT") {
            std::cerr << "Invalid operation" << std::endl;
            return ClientType::INVALID;
        }
        string result = utils::getPayload(response);
        return StringToClientType.at(result);
    }

    bool isOperationValid(const string& operation) {
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
            string command;
            std::getline(std::cin, command);
            command = utils::trim(command);  // Remove leading and trailing spaces
            // 1. Check if the command is "exit"
            if (command == "exit") {
                break;
            }
            // 2. Check if the command is valid for the client type
            string operation = utils::getOperation(command);
            if (!isOperationValid(operation)) {
                std::cerr << "Invalid operation: " << operation << std::endl;
                continue;
            }
            // 3. If the command is valid, add the username and send it to the server
            string payload = utils::getPayload(command);
            string data = utils::join({operation, this->username, payload});
            client->send(command);
        }
    }
};

Client* global_client = nullptr;

void handleSigint(int signal) {
    std::cout << "\nCaught signal " << signal << ". Shutting down client..." << std::endl;
    delete global_client; // Ensures the destructor is called
    std::exit(0);
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

