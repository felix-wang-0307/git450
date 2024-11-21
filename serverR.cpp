#include <iostream>
#include "lib/utils.h"

int PORT = std::stoi(config["server_r_port"]);

void bootUp() {
    std::cout << "Server R is up and running using UDP on port " << PORT << std::endl;
};

int main() {
    bootUp();
    return 0;
}