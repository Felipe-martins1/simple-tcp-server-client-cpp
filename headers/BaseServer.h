#ifndef BASESERVER_H
#define BASESERVER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <cctype>
#include <locale>
#include <fcntl.h>
#include <vector>

#include "SharedMemoryManager.h"
#include "ServerClient.h"
#include "./utils/QueryUtils.h"
#include "./utils/DatabaseUtils.h"

int port1 = 8080;
int port2 = 8081;
int port3 = 8082;

int availableServers[3] = {port1, port2, port3};

std::string handleQuery(const std::string& query, int port) {
    QueryUtils queryUtils;
    DatabaseUtils databaseUtils;

    Query* parsedQuery = queryUtils.getQuery(query);

    if(parsedQuery == nullptr){
        return "Invalid query format. " + queryUtils.availableFormats() + "\n";
    }

    std::string name = parsedQuery->search;

    if (name.empty()) {
        return "Name cannot be empty. " + queryUtils.availableFormats() + "\n";
    }

    std::string response;
    SharedMemoryManager sharedMemoryManager;

    if(parsedQuery->isGetAllInServer) {
        sharedMemoryManager.startSharedMemory();
        sharedMemoryManager.addToSharedMemory(databaseUtils.findOneInServer(port, ""));
    }

    if (parsedQuery->isGetAllInAllServers) {
        sharedMemoryManager.startSharedMemory();
        sharedMemoryManager.clear();
        for (int serverPort : availableServers) {
            if(serverPort == port) {
                sharedMemoryManager.addToSharedMemory(databaseUtils.findOneInServer(port, ""));
            }else{
                ServerClient client2("127.0.0.1", serverPort);
                client2.sendQuery("GET *");
            }
        }

        response = sharedMemoryManager.getSharedData()->sharedString;
        sharedMemoryManager.clear();
        return response;
    }

    
    if(parsedQuery->isSearchInAllServers) {
        
        response = databaseUtils.findInAllServers(availableServers, sizeof(availableServers) / sizeof(availableServers[0]), port, name);

        if(response.empty()){
            return "NOT FOUND";
        }

        return response;
    }

    if(parsedQuery->isSearchInServer) {
        response = databaseUtils.findOneInServer(port, name);
    }

    if(response.empty()){
        return "NOT FOUND";
    }

    return response;
}

class BaseServer {
public:
    void start(int port) {
        int serverSocket;
        
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            std::cerr << "Failed to create socket for server " << std::endl;
            return;
        }

        
        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            std::cerr << "Failed to bind socket for server " << std::endl;
            close(serverSocket);
            return;
        }

        // Listen for incoming connections
        if (listen(serverSocket, 5) == -1) {
            std::cerr << "Failed to listen for connections on server " << std::endl;
            close(serverSocket);
            return;
        }

        std::cout << "Server " << " started. Listening for connections on port " << port << "..." << std::endl;
    
        while (true) {
            fd_set readSet;
            FD_ZERO(&readSet);
            int maxSocket = -1;
            FD_SET(serverSocket, &readSet);
            maxSocket = std::max(maxSocket, serverSocket);
            

            if (select(maxSocket + 1, &readSet, nullptr, nullptr, nullptr) == -1) {
                std::cerr << "Error in select()" << std::endl;
                break;
            }

            if (FD_ISSET(serverSocket, &readSet)) {
                // Accept incoming connection
                sockaddr_in clientAddress{};
                socklen_t clientAddressLength = sizeof(clientAddress);
                int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
                if (clientSocket == -1) {
                    std::cerr << "Failed to accept connection on server " << std::endl;
                    continue;
                }

                std::cout << "Client connected to server " << std::endl;

                // Handle client request
                char buffer[1024];
                ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead == -1) {
                    std::cerr << "Failed to receive data from client on server " << std::endl;
                    close(clientSocket);
                    continue;
                }

                std::cout << "Received query from client on server" << ": " << buffer << std::endl;

                std::string query(buffer, bytesRead);
                std::string response = handleQuery(query, port);
                ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
                if (bytesSent == -1) {
                    std::cerr << "Failed to send response to client on server" << std::endl;
                    close(clientSocket);
                    continue;
                }

                std::cout << "Response sent to client on server: \n" << std::endl;
                // Close the client socket
                close(clientSocket);
            }
            
        }
    }
};

#endif