#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <limits>

#include "./headers/ServerClient.h"

int main() {
    int port;

    std::cout << "Enter the port: ";
    std::cin >> port;

    while (true) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        std::cout << "Enter the query: ";
        std::getline(std::cin, query);

        
        ServerClient client("127.0.0.1", serverPort);
        std::string response = client.sendQuery(query);

        std::cout << "Response from server: " << response << std::endl;

        std::cout << "Do you want to send another query? (y/n): ";
        char choice;
        std::cin >> choice;
        if (choice != 'y' && choice != 'Y') {
            break;
        }
    }

    return 0;
}
