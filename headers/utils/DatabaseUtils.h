#ifndef DATABASEUTILS_H_H
#define DATABASEUTILS_H_H

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

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");

    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }

    return str.substr(start, end - start + 1);
}

bool caseInsensitiveStringCompare(const std::string& str1, const std::string& str2) {
    return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(),
                      [](char c1, char c2) {
                          return std::tolower(c1) == std::tolower(c2);
                      });
}



class DatabaseUtils {
public:
    std::string findOneInServer(int port, const std::string& search){
        std::ifstream file;
        file.open("databases/" + std::to_string(port) + ".txt");
        
        if(!file.is_open()){
            return "";
        }

        std::string response;

        std::string line;
        
        while(std::getline(file, line)){
            if(search.empty()){
                response += line + "\n";
            }else{
                std::string trimmedLine = trim(line);
                std::string trimmedSearch = trim(search);
                if (caseInsensitiveStringCompare(trimmedLine, trimmedSearch)) {
                    response = line + "\n";
                    break;
                }
            }
        }

        file.close();

        return response;
    }

    std::string findInAllServers(int* ports, int size, int currentPort, const std::string& search){
        std::string response;
        int pipe_fd[2];
        
        response = findOneInServer(currentPort, search);

        if(response.empty()){
            if (pipe(pipe_fd) == -1) {
                return "Failed to create pipe.\n";
            }

            for (int i = 0; i < size; ++i) {
                int serverPort = ports[i];
            
                if (serverPort == currentPort) {
                    response = findOneInServer(serverPort, search);
                    continue;
                }

                pid_t pid = fork();
                if (pid == -1) {
                    return "Failed to fork process.\n";
                } else if (pid == 0) { // É um processo filho
                    close(pipe_fd[0]); // Fecha leitura do pipe no filho

                    // Cria um cliente para se conectar ao servidor
                    ServerClient client("127.0.0.1", serverPort);
                    std::string resQuery = client.sendQuery("GET " + search);
                    if (resQuery != "NOT FOUND") {
                        write(pipe_fd[1], resQuery.c_str(), resQuery.size() + 1); // Escreve no pipe
                    }
                    close(pipe_fd[1]); // Fecha escrita do pipe no filho
                    _exit(0); // Encerra o processo filho
                }
            }

            close(pipe_fd[1]); // Fecha escrita do pipe no pai
            char buffer[1024];
            ssize_t bytesRead = read(pipe_fd[0], buffer, sizeof(buffer)); // Lê o resultado do pipe
            if (bytesRead != -1) {
                response = std::string(buffer, bytesRead);
                close(pipe_fd[0]); // Fecha leitura do pipe no pai
            }
        }

        return response;
    }
};

#endif