#include <iostream>
#include <core/window.hpp>

int main(int argc, char** argv) {
    window window(800,600,"Cytos");
    std::cout << "Hello cytos" << std::endl;

    while (!window.shouldClose()) {
        window.pollEvents();
        window.swapBuffers();
    }

    return 0;
}