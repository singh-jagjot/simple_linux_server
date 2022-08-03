//
// Created by jj on 23/07/22.
//
#include <vector>
#include <cstring>
#include <poll.h>
#include "../include/common.h"
#include "../include/peer_handler.h"
#include "../include/string_helper.h"
#include "../include/db_file_util.h"

bool is_master = false;
std::vector<std::string> peers_add_vec;
std::vector<int> peers_port_vec;

int handle_peer(const int peer_fd, std::vector<std::string> &peers_add, std::vector<int> &peers_port){
    peers_add_vec = peers_add;
    peers_port_vec = peers_port;
    char read_buffer[BUFFER_SIZE] = {0};
    char response_buffer[BUFFER_SIZE] = {0};
    bool in_progress = false;
    std::vector<std::string> peer_msgs;

    while (true) {
        bzero(response_buffer, BUFFER_SIZE);
        long bytes_read = read(peer_fd, read_buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            printf("Error while reading the read_buffer. Error No: %i", errno);
            exit(EXIT_FAILURE);
        }
        if (bytes_read == 0){
            printf("Peer exited abruptly\n");
            break;
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
        } else if(string_startswith(read_buffer, QUIT)){
            printf("Peer exiting\n");
            break;
        }{
            sprintf(response_buffer,"Invalid cmd: %s\n", read_buffer);
        }
        write(peer_fd, response_buffer, BUFFER_SIZE);
        printf("Bytes read: %li, Message: %s", bytes_read, read_buffer);
        bzero(read_buffer, BUFFER_SIZE);
    }
    return 1;
}

void master_set(){
    is_master = true;
}

void master_unset(){
    is_master = false;
}

void wait(){
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(SYNC_TIMEOUT);
}

int write_operation(std::string &msg){
    master_set();
    std::string final_msg = msg;
    for(int i = 0; i< peers_add_vec.size(); ++i){
        auto add = peers_add_vec[i];
        auto port = peers_port_vec[i];
        if(send_to_peer(add, port, final_msg) == -1){
            return -1;
        }
    }
    return 0;
}