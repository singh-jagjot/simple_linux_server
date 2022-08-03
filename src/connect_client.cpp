//
// Created by jj on 22/07/22.
//
#include "../include/connect_client.h"
#include <thread>
#include <vector>
#include <unistd.h>

int socket_fd;

void create_socket_connection(std::string address, std::string is_daemon, int port, int max_threads, const std::string &file_path,
                              int (*handle)(const int &socket_fd, const std::string &file_path, sockaddr_in &socket_add)) {
    // if(!strcmp(is_daemon.data(), "true")){

    
    //    Creating a IPv4 and TCP socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("Failed to create TCP socket. Error No: %i\n", errno);
        exit(EXIT_FAILURE);
    }

    // Setting option to reuse the socket
    int reuse = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &reuse, sizeof(reuse));

    //Attaching socket to the given port
    int connection_port = port;
    sockaddr_in socket_add{};
    socket_add.sin_family = AF_INET;
    if(!address.empty()){
        auto add = inet_addr(address.data());
        if(add == -1){
            printf("Invalid IP Address: %s\n",address.data());
            exit(EXIT_FAILURE);
        }
        socket_add.sin_addr.s_addr =  add;
    } else{
        socket_add.sin_addr.s_addr = INADDR_ANY;
    }
    socket_add.sin_port = htons(connection_port);

    int socket_add_len = sizeof(socket_add);
    if (bind(socket_fd, (struct sockaddr *) &socket_add, socket_add_len) < 0) {
        printf("Binding to port %i failed. Error No: %i\n", connection_port, errno);
        exit(EXIT_FAILURE);
    }

    //Start listening the socket and hold max_threads connections
    if (listen(socket_fd, max_threads) < 0) {
        printf("Listening on the socket failed. Error No: %i\n", errno);
        exit(EXIT_FAILURE);
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < max_threads; ++i) {
        threads.emplace_back(connection,std::ref(is_daemon), std::ref(socket_fd), std::ref(socket_add), std::ref(socket_add_len),
                             std::ref(file_path), std::ref(handle));
    }

    for (auto &t: threads) {
        t.join();
    }
}

void connection(std::string is_daemon, int &socket_fd, sockaddr_in &socket_add, int &socket_add_len, const std::string &file_path,
                int (*handle)(const int &socket_fd, const std::string &file_path, sockaddr_in &socket_add)) {
//  Grabbing a connection from the listening queue
    while (true) {
        int new_socket = accept(socket_fd, (struct sockaddr *) &socket_add, (socklen_t *) &socket_add_len);
        if (new_socket < 0) {
            printf("Grabbing a connection failed. Error No: %i\n", errno);
//            exit(EXIT_FAILURE);
            sleep(1);
            continue;
        }
        // Setting option to reuse the socket
        int reuse = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &reuse, sizeof(reuse));

        handle(new_socket, file_path, socket_add);

//      closing the connected socket
        shutdown(new_socket, SHUT_RDWR);
        close(new_socket);
    }
}

void master_socket_close(){
//  closing the listening socket
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}