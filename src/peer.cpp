//
// Created by jj on 23/07/22.
//
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/peer.h"
#include "../include/string_helper.h"
#include "../include/db_file_util.h"

int peer_socket_fd;
bool is_master = false;

int handle_peer(const int& socket_fd, const std::string& file_path, sockaddr_in &socket_add){

    peer_socket_fd = socket_fd;
    char read_buffer[BUFFER_SIZE] = {0};
    char response_buffer[BUFFER_SIZE] = {0};
    std::vector<std::string> client_msgs;
    printf("Connected Peer Address: %s:%i\n", inet_ntoa(socket_add.sin_addr), ntohs(socket_add.sin_port));

    while (true) {
        bzero(response_buffer, BUFFER_SIZE);
        long bytes_read = read(socket_fd, read_buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            printf("Error while reading the read_buffer. Error No: %i", errno);
            exit(EXIT_FAILURE);
        }
        if(string_startswith(read_buffer, SYNC_START)) {
            try {
                lock_acquire();
                printf("%s: lock acquired\n", SYNC_ACK_OK);
                sprintf(response_buffer, "%s", SYNC_ACK_OK);
            } catch (...) {
                printf("Sync acknowledgement: FAILED\n");
                sprintf(response_buffer, "%s", SYNC_ACK_FAIL);
            }
        } else if (string_startswith(read_buffer, SYNC_ACK_FAIL)){

        } else if(string_startswith(read_buffer, SYNC_ACK_OK)){

        } else if(string_startswith(read_buffer, SYNC_ABORT)){
            lock_release();
            printf("%s: lock released\n", SYNC_ABORT);
        } else if(string_startswith(read_buffer, SYNC_COMMIT)){

        } else if(string_startswith(read_buffer, SYNC_COMMIT_FAIL)){

        } else if(string_startswith(read_buffer, SYNC_COMMIT_OK)){

        } else if(string_startswith(read_buffer, SYNC_FAILED)){

        } else if(string_startswith(read_buffer, SYNC_COMPLETED)){
            lock_release();
            printf("%s: lock released\n", SYNC_COMPLETED);
        } else{
            sprintf(response_buffer,"Invalid cmd: %s\n", read_buffer);
        }
        write(socket_fd, response_buffer, BUFFER_SIZE);
        printf("Bytes read: %li, Message: %s", bytes_read, read_buffer);
        bzero(read_buffer, BUFFER_SIZE);
    }
    return 1;
}

long send_to_peer(std::string msg){
    return write(peer_socket_fd, msg.data(), msg.size());
}

void master_set(){
    is_master = true;
}

void master_unset(){
    is_master = false;
}