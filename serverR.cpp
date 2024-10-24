#include <iostream>
#include "serverR.h"

void bootUp() {
    std::cout << "Server R is up and running using UDP on port " << PORT << std::endl;
};

int main() {
    bootUp();
    return 0;
}