//
// Created by jj on 22/07/22.
//
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include "../include/common.h"
#include "../include/client_handler.h"
#include "../include/connect_client.h"

int master_socket_client;
static bool is_debug = false;

//Method to create the master client socket to accept
//client for communication.
void create_socket_client(std::map<std::string, std::string> &args) {
    if(!strcmp(args[DEBUG].data(), "1")){
        is_debug = true;
    }
    //    Creating a IPv4 and TCP socket
    master_socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if (master_socket_client == -1) {
        printf("Failed to create TCP socket. Error No: %i\n", errno);
        exit(EXIT_FAILURE);
    }

    // Setting option to reuse the socket
    int reuse = 1;
    setsockopt(master_socket_client, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &reuse, sizeof(reuse));

    //Attaching socket to the given port
    int connection_port = std::stoi(args[BBPORT]);
    sockaddr_in socket_add{};
//    if(!address.empty()){
//        auto add = inet_addr(address.data());
//        if(add == -1){
//            printf("Invalid IP Address: %s\n",address.data());
//            exit(EXIT_FAILURE);
//        }
//        socket_add.sin_addr.s_addr =  add;
//    } else{
//        socket_add.sin_addr.s_addr = INADDR_ANY;
//    }
    socket_add.sin_family = AF_INET;
    socket_add.sin_addr.s_addr = INADDR_ANY;
    socket_add.sin_port = htons(connection_port);

    int socket_add_len = sizeof(socket_add);
    if (bind(master_socket_client, (struct sockaddr *) &socket_add, socket_add_len) < 0) {
        printf("Binding to port %i failed. Error No: %i\n", connection_port, errno);
        exit(EXIT_FAILURE);
    }

    int max_threads = std::stoi(args[THMAX]);
    //Start listening the socket and hold max_threads connections
    if (listen(master_socket_client, max_threads) < 0) {
        printf("Listening on the socket failed. Error No: %i\n", errno);
        exit(EXIT_FAILURE);
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < max_threads; ++i) {
        threads.emplace_back(accept_clients, std::ref(master_socket_client), std::ref(socket_add),
                             std::ref(socket_add_len), std::ref(args));
    }

    for (auto &t: threads) {
        t.join();
    }
}

// To keep on accepting new clients.
[[noreturn]] void accept_clients(int &socket_fd, sockaddr_in &socket_add, int &socket_add_len, std::map<std::string, std::string> &args) {
//  Grabbing a accept_clients from the listening queue
    while (true) {
        int new_socket = accept(socket_fd, (struct sockaddr *) &socket_add, (socklen_t *) &socket_add_len);
        if (new_socket == -1) {
            printf("Grabbing a accept_clients failed. Error No: %i\n", errno);
//            exit(EXIT_FAILURE);
            sleep(1); //To make sure program doesn't exit after receiving SIGHUP.
            continue;
        }
        // Setting option to reuse the socket
        int reuse = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &reuse, sizeof(reuse));
        if(is_debug){
            printf("Client connected\n");
        }
        handle_client(new_socket, args);

//      closing the connected socket
        shutdown(new_socket, SHUT_RDWR);
        close(new_socket);
        if(is_debug){
            printf("Client disconnected\n");
        }
    }
}

void master_socket_close(){
//  closing the listening socket
    if(is_debug){
        if(shutdown(master_socket_client, SHUT_RD) == -1){
            printf("Error while shutting down the master_socket_client\n");
        } else {
            printf("Success: Shutting down the master_socket_client\n");
        }
        if(close(master_socket_client) == -1){
            printf("Error while closing the master_socket_client\n");
        } else{
            printf("Success: Closing the master_socket_client\n");
        }
    }
}