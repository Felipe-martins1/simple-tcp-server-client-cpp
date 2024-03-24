# Simple TCP Server-Client in C++

This is a simple TCP server-client implementation in C++. It demonstrates how to establish a TCP connection between a server and a client, and exchange messages over the network.

The project involves the operation of three servers, primarily focusing on two main operations: findAll and findOne.

In the findAll operation, the server is tasked with searching for all information across the three servers and storing it in shared memory. Access to this shared memory is restricted through the implementation of a semaphore.

On the other hand, the findOne operation involves searching for a specific piece of data on one of the three servers using pipes.

## Prerequisites

Before running the code, make sure you have the following:

- C++ compiler (e.g., GCC or Clang)
- Basic understanding of TCP/IP networking concepts

## Usage

1. Clone the repository:

2. Compile the server and client programs:

    ```shell
    g++ server.cpp -o server
    g++ client.cpp -o client
    ```

3. Start the server 1(Port: 8080):

    ```shell
    ./server
    ```

4. Start the server 2(Port: 8081):

    ```shell
    ./server
    ```

5. Start the server 3(Port: 8082):

    ```shell
    ./server
    ```

4. Run the client:

    ```shell
    ./client
    ```

5. Follow the prompts in the client program to send and receive messages.

## Contributing

Contributions are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).
