//
// Created by jj on 02/08/22.
//

#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "../include/common.h"
#include "../include/peer_notify.h"

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
