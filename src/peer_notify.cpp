//
// Created by jj on 02/08/22.
//

#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "../include/common.h"
#include "../include/peer_notify.h"
#include "../include/string_helper.h"

long send_to_peer(std::string ip, int port, std::string msg){
    try {
        //    Creating a IPv4 and TCP socket
        int peer_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (peer_socket == -1) {
            printf("Failed to create TCP socket. Error No: %i\n", errno);
            exit(EXIT_FAILURE);
        }

        // Setting option to reuse the socket
        int reuse = 1;
        setsockopt(peer_socket, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in socket_add{};
        auto binary_add = inet_addr(ip.data());
        if(binary_add == -1){
            printf("Invalid IP Address: %s\n", ip.data());
            exit(EXIT_FAILURE);
        }

        socket_add.sin_family = AF_INET;
        socket_add.sin_addr.s_addr = binary_add;
        socket_add.sin_port = htons(port);
        int socket_add_len = sizeof(socket_add);

        char response_buffer[BUFFER_SIZE] = {0};
        sprintf(response_buffer, "%s\n", msg.data());
        if(connect(peer_socket, (struct sockaddr *) &socket_add, socket_add_len) !=0){
            printf("Failed to connect with the peer: %s:%i\n", ip.data(), port);
            return -1;
        }
        printf("Connected to the peer: %s:%i\n", ip.data(), port);
        long bytes_written = write(peer_socket, response_buffer, BUFFER_SIZE);
        printf("%li bytes wrote on socket of peer: %s:%i TEXT: %s", bytes_written, ip.data(), port, response_buffer);
        printf("Quiting...");
        bzero(response_buffer, BUFFER_SIZE);
        sprintf(response_buffer, "%s\n", QUIT);
        write(peer_socket, response_buffer, strlen(QUIT));
        shutdown(peer_socket, SHUT_RDWR);
        close(peer_socket);
        printf("Peer disconnected at %s:%i\n", ip.data(), port);
//        printf("Returning %li\n", bytes_written);
        return bytes_written;
    } catch (...) {
        return -1;
    }
}

int get_receiver_fd(int port){
    try {
        //    Creating a IPv4 and TCP socket
        int receiving_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (receiving_socket == -1) {
            printf("Failed to create TCP socket. Error No: %i\n", errno);
            exit(EXIT_FAILURE);
        }

        // Setting option to reuse the socket
        int reuse = 1;
        setsockopt(receiving_socket, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in socket_add{};
        socket_add.sin_family = AF_INET;
        socket_add.sin_addr.s_addr = INADDR_ANY;
        socket_add.sin_port = htons(port);
        int socket_add_len = sizeof(socket_add);

        if (bind(receiving_socket, (struct sockaddr *) &socket_add, socket_add_len) < 0) {
            printf("Binding to port %i failed. Error No: %i\n", port, errno);
            exit(EXIT_FAILURE);
        }
        printf("Binding to port %i successful.\n", port);

        //Start listening the socket and hold max connection requests
        if (listen(receiving_socket, 1) < 0) {
            printf("Listening(preparing to listen) on the socket failed. Error No: %i\n", errno);
            exit(EXIT_FAILURE);
        }
        printf("Listening(preparing to listen) on the receiving socket %i successful.\n", receiving_socket);

        int peer_fd = accept(receiving_socket, (struct sockaddr *) &socket_add, (socklen_t *) &socket_add_len);

        return peer_fd;
    } catch (...) {
        return -1;
    }
}

long send_to_all(std::set<std::string> &peers, const std::string& msg) {
    bool is_fail = false;
    for(auto &peer: peers){
        auto vec = string_split(peer, ':');
        auto ip = vec[0];
        auto port = std::stoi(vec[1]);
        if(send_to_peer(ip, port, msg) == -1){
            printf("Sending '%s' msg failed for peer: %s:%i\n", msg.data(), ip.data(), port);
            is_fail = true;
        }
    }
    if(is_fail){
        return -1;
    }
    return 0;
}