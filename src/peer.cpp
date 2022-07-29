//
// Created by jj on 23/07/22.
//
#include <vector>
#include <cstring>
#include <unistd.h>
#include "../include/peer.h"

int peer_socket_fd;
int handle_peer(const int& socket_fd, const std::string& file_path){
//    inet_addr

    peer_socket_fd = socket_fd;
    char read_buffer[BUFFER_SIZE] = {0};
    char response_buffer[BUFFER_SIZE] = {0};
    std::vector<std::string> client_msgs;
//    write(socket_fd, RES_INITIAL_GREETING, sizeof(RES_INITIAL_GREETING));
    bool replicated = false;

    while (true) {
        bzero(response_buffer, BUFFER_SIZE);
        long bytes_read = read(socket_fd, read_buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            printf("Error while reading the read_buffer. Error No: %i", errno);
            exit(EXIT_FAILURE);
        }
    }
    return 1;
}

long send_to_peer(std::string msg){
    return write(peer_socket_fd, msg.data(), msg.size());
}