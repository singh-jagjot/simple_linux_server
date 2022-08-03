//
// Created by jj on 02/08/22.
//

#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "../include/common.h"
#include "../include/peer_notify.h"

long send_to_peer(std::string add, int port, std::string msg){
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
    auto binary_add = inet_addr(add.data());
    if(binary_add == -1){
        printf("Invalid IP Address: %s\n",add.data());
        exit(EXIT_FAILURE);
    }

    socket_add.sin_family = AF_INET;
    socket_add.sin_addr.s_addr = binary_add;
    socket_add.sin_port = htons(port);
    int socket_add_len = sizeof(socket_add);

    if(connect(peer_socket, (struct sockaddr *) &socket_add, socket_add_len) !=0){
        printf("Failed to connect with the peer: %s:%i\n", add.data(), port);
        return -1;
    }
    printf("Connected to the peer: %s:%i\n", add.data(), port);
    auto bytes_written = write(peer_socket, msg.data(), msg.size());
    write(peer_socket, QUIT, strlen(QUIT));
    shutdown(peer_socket, SHUT_RDWR);
    close(peer_socket);
    printf("Peer disconnected. Address: %s:%i\n", add.data(), port);
    return bytes_written;
}
