#include <iostream>
#include "client.h"
#include "lib/tcpSocket.h"
#include "lib/utils.h"

enum ClientType {
    GUEST,
    USER
};

class Client {
public:
    TCPClientSocket* client;
    int type;
    Client(std::string username, std::string password) {
        bootUp();
        type = authenticate(username, password);
        if (type == -1) {
            std::cerr << "Failed to authenticate" << std::endl;
            exit(1);
        }
    }
    ~Client() {
        delete client;
    }
    void bootUp() {
        client = new TCPClientSocket();
        std::cout << "Client is up and running" << std::endl;
    }
    int authenticate(const std::string& username, const std::string& password) {
        std::string data = "AUTH " + username + " " + password;
        client->send(data);
        std::string response = client->receive();
        if (response == "USER") {
            return USER;
        } else if (response == "GUEST") {
            return GUEST;
        } else {
            return -1;
        }
    }
    void run() {
        while (true) {

        }
    }
};

int main(int argc, char* argv[]) {
    // Run the client: ./client <username> <password>
    if (argc < 3) {
        std::cerr << "Usage: ./client <username> <password>" << std::endl;
        exit(1);
    }
    Client client(argv[1], argv[2]);
    return 0;
}

