#include <iostream>
#include "lib/utils.h"
#include "lib/config.h"

int PORT = config::SERVER_R_PORT;

void bootUp() {
    std::cout << "Server R is up and running using UDP on port " << PORT << std::endl;
};

int main() {
    bootUp();
    return 0;
}