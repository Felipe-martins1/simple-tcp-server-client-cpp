
#ifndef SERVERCLIENT_H
#define SERVERCLIENT_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

class ServerClient {
public:
    ServerClient(const std::string& serverIP, int serverPort) : serverIP(serverIP), serverPort(serverPort) {}

    std::string sendQuery(const std::string& query) {
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return "";
        }

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(serverPort);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);

        if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            std::cerr << "Failed to connect to server" << std::endl;
            close(clientSocket);
            return "";
        }

        ssize_t bytesSent = send(clientSocket, query.c_str(), query.length(), 0);
        if (bytesSent == -1) {
            std::cerr << "Failed to send query to server" << std::endl;
            close(clientSocket);
            return "";
        }

        char buffer[2048];
        ssize_t totalBytesRead = 0;
        ssize_t bytesRead;

        while ((bytesRead = recv(clientSocket, buffer + totalBytesRead, sizeof(buffer) - totalBytesRead - 1, 0)) > 0) {
            totalBytesRead += bytesRead;
            if (totalBytesRead >= sizeof(buffer) - 1) break;
        }

        if (bytesRead == -1) {
            std::cerr << "Failed to receive response from server" << std::endl;
            close(clientSocket);
            return "";
        }

        buffer[totalBytesRead] = '\0';
        std::string response(buffer);

        close(clientSocket);
        return response;
    }

private:
    std::string serverIP;
    int serverPort;
};

#endif