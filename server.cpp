#include "./headers/BaseServer.h"

int main() {
    BaseServer server;
    std::cout << "Enter the port: ";
    int port;
    std::cin >> port;
    server.start(port);
}
