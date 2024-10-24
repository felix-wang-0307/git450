#include <iostream>
#include "serverM.h"

void bootUp() {
    std::cout << "Server M is up and running using UDP on port " << UDP_PORT << std::endl;
};

int main() {
    bootUp();
    return 0;
}