#ifndef SHAREDMEMORYMANAGER_H
#define SHAREDMEMORYMANAGER_H

#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <semaphore.h>

struct SharedData {
    int sharedValue;
    char sharedString[256];
};

class SharedMemoryManager {
private:
    int shm_fd;
    void* shared_memory;
    SharedData* shared_data;
    sem_t* semaphore; // Semáforo para controlar o acesso à memória compartilhada

public:
    SharedMemoryManager() : shm_fd(-1), shared_memory(nullptr), shared_data(nullptr), semaphore(nullptr) {
        // Inicializa o semáforo
        semaphore = sem_open("/semaphore", O_CREAT, 0644, 1);
        if (semaphore == SEM_FAILED) {
            std::cerr << "Failed to create semaphore" << std::endl;
        }
    }

    ~SharedMemoryManager() {
        // Libera os recursos alocados

        if (shared_memory != nullptr) {
            munmap(shared_memory, sizeof(SharedData));
        }

        if (shm_fd != -1) {
            close(shm_fd);
        }

        // Fecha o semáforo
        sem_close(semaphore);
        sem_unlink("/my_semaphore");
    }

    bool startSharedMemory() {
        const int SHARED_MEMORY_SIZE = sizeof(SharedData);

        shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            std::cerr << "Failed to open shared memory" << std::endl;
            return false;
        }

        if (ftruncate(shm_fd, SHARED_MEMORY_SIZE) == -1) {
            std::cerr << "Failed to set the size of shared memory" << std::endl;
            return false;
        }

        shared_memory = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shared_memory == MAP_FAILED) {
            std::cerr << "Failed to map shared memory" << std::endl;
            return false;
        }

        shared_data = static_cast<SharedData*>(shared_memory);

        return true;
    }

    bool writeSharedMemory(const std::string& str) {
        if (shared_data == nullptr) {
            std::cerr << "Shared memory not initialized" << std::endl;
            return false;
        }

        // Tenta adquirir o semáforo em um loop até conseguir
        while (sem_trywait(semaphore) == -1) {
            if (errno != EINTR) {
                std::cerr << "Failed to acquire semaphore" << std::endl;
                return false;
            }
        }

        shared_data->sharedValue = 1;

        if (str.length() >= sizeof(shared_data->sharedString)) {
            std::cerr << "String is too long to write to shared memory" << std::endl;
            // Libera o semáforo antes de retornar
            sem_post(semaphore);
            return false;
        }

        std::strcpy(shared_data->sharedString, str.c_str());
        
        // Libera o semáforo após escrever na memória compartilhada
        sem_post(semaphore);
        
        return true;
    }

    bool addToSharedMemory(const std::string& str) {
        if (shared_data == nullptr) {
            std::cerr << "Shared memory not initialized" << std::endl;
            return false;
        }

        // Tenta adquirir o semáforo até conseguir
        while (sem_trywait(semaphore) == -1) {
            if (errno != EINTR) {
                std::cerr << "Failed to acquire semaphore" << std::endl;
                return false;
            }
        }

        shared_data->sharedValue += 1;

        if (str.length() + std::strlen(shared_data->sharedString) >= sizeof(shared_data->sharedString)) {
            std::cerr << "String is too long to write to shared memory" << std::endl;
            // Libera o semáforo antes de retornar
            sem_post(semaphore);
            return false;
        }

        std::strcat(shared_data->sharedString, str.c_str());
        
        // Libera o semáforo após escrever na memória compartilhada
        sem_post(semaphore);
        
        return true;
    }

    void clear() {
        if (shared_data == nullptr) {
            std::cerr << "Shared memory not initialized" << std::endl;
            return;
        }

        // Tenta adquirir o semáforo até conseguir
        while (sem_trywait(semaphore) == -1) {
            if (errno != EINTR) {
                std::cerr << "Failed to acquire semaphore" << std::endl;
                return;
            }
        }

        shared_data->sharedValue = 0;
        shared_data->sharedString[0] = '\0';
        
        // Libera o semáforo após escrever na memória compartilhada
        sem_post(semaphore);
    }

    SharedData* getSharedData() const {
        return shared_data;
    }
};

#endif