#include <iostream>
#include "serverA.h"

void bootUp() {
    std::cout << "Server A is up and running using UDP on port " << PORT << std::endl;
};

bool authenticate() {
    std::cout << "Server A is authenticating the client" << std::endl;
    return true;
};

int main() {
    bootUp();
    return 0;
}