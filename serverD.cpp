#include <iostream>
#include "serverD.h"

void bootUp() {
    std::cout << "Server D is up and running using UDP on port " << PORT << std::endl;
};

int main() {
    bootUp();
    return 0;
}