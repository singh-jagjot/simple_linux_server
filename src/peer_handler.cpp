//
// Created by jj on 23/07/22.
//
#include <vector>
#include <cstring>
#include <poll.h>
#include <stdexcept>
#include "../include/common.h"
#include "../include/peer_handler.h"
#include "../include/string_helper.h"
#include "../include/db_file_util.h"
#include "../include/peer_notify.h"

bool is_master = false;
int pfd;
std::vector<std::string> peers_add_vec;
std::vector<int> peers_port_vec;
std::string my_add;
std::string my_ip;
std::string my_port;
std::string master_add;
std::string master_ip;
int master_port;
std::string file_path;

void set_peers(std::string &peers){
    std::vector<std::string> p = string_split(peers, ' ');
    std::vector<std::string> peers_add;
    std::vector<int> peers_port;
    for (auto &peer: p) {
        auto ipv4_add = string_split(peer, ':');
        if (ipv4_add.size() != 2) {
            printf("Wrong IP address: %s\n", peer.data());
            exit(EXIT_FAILURE);
        }
        peers_add.push_back(ipv4_add[0]);
        peers_port.push_back(std::stoi(ipv4_add[1]));
        printf("Peer added: %s:%s\n", ipv4_add[0].data(),ipv4_add[1].data());
    }

    peers_add_vec = peers_add;
    peers_port_vec = peers_port;
}

long get_peer_count(){
    return peers_add_vec.size();
}
void set_my_add(std::string &iadd){
    my_add = iadd;
    auto ip_vec = string_split(iadd, ':');
    my_ip = ip_vec[0];
    my_port = ip_vec[1];
}

int handle_peer(const int peer_fd, std::map<std::string, std::string> &args){
    pfd = peer_fd;
    printf("Socket: %i\n", pfd);
    char read_buffer[BUFFER_SIZE] = {0};
    char response_buffer[BUFFER_SIZE] = {0};
    bool in_progress = false;
    file_path = args[BBFILE];
    std::vector<std::string> peer_msgs;

    while (true) {
        bzero(response_buffer, BUFFER_SIZE);
        long bytes_read = read(peer_fd, read_buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            printf("Error while reading the read_buffer. Error No: %i\n", errno);
            exit(EXIT_FAILURE);
        }
        if (bytes_read == 0){
            printf("Peer exited abruptly\n");
            break;
        }
        printf("Bytes read: %li, Message: %s", bytes_read, read_buffer);

        if(string_startswith(read_buffer, SYNC_START)) {
            try {
                in_progress = true;
                master_add = string_split(read_buffer, '|')[1];
                auto v = string_split(master_add, ':');
                master_ip = v[0];
                master_port = std::stoi(v[1]);

                lock_acquire();
                printf("lock acquired\n");
//                printf(SYNC_ACK_OK, my_add.data(), read_recent_line_number(file).data());
//                sprintf(response_buffer, SYNC_ACK_OK, my_add.data(), read_recent_line_number(file).data());
                if(send_to_peer(master_ip, master_port, ready_response(SYNC_ACK_OK)) == -1){
                    throw std::runtime_error("Failed to send acknowledgement\n");
                }
            } catch (...) {
                printf("Sync acknowledgement: FAILED\n");
//                bzero(response_buffer, BUFFER_SIZE);
//                sprintf(response_buffer, SYNC_ACK_FAIL, my_add.data());
                send_to_peer(master_ip, master_port, ready_response(SYNC_ACK_FAIL));
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
        } else {
            sprintf(response_buffer,"Invalid cmd: %s\n", read_buffer);
        }
//        write(peer_fd, response_buffer, BUFFER_SIZE);
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

//Step 1: Notify for sync
int sync_step1(){
    printf("Peers Size: %zu\n", peers_add_vec.size());
    for(int i = 0; i< peers_add_vec.size(); ++i){
        auto add = peers_add_vec[i];
        auto port = peers_port_vec[i];
        if(send_to_peer(add, port, ready_response(SYNC_START)) == -1){
            return -1;
        }
    }
//    printf("Step 1 returned 0\n");
    return 0;
}

//Step 2: Wait for acknowledgement
int sync_step2(){
    struct pollfd p[1];
    p[0].fd = pfd;
    p[0].events = POLLIN;
    printf("Monitored FD: %i\n", pfd);
    char read_buffer[BUFFER_SIZE] = {0};
    int events_num = poll(p, 1, 10000);
    if(events_num == 0){
        printf("Wait timed out for acknowledgments\n");
    } else {
        int pollin_happened = p[0].revents & POLLIN;
        if(pollin_happened){
            long bytes_read = read(p[0].fd, read_buffer, BUFFER_SIZE);
            if (bytes_read < 0) {
                printf("Error while reading the read_buffer. Error No: %i\n", errno);
                return -1;
            }
            printf("Polled: %s\n", read_buffer);
            printf("Step 2 returning: 0\n");
            return 0;
        } else {
            printf("System error occurred: %d\n", p[0].revents);
        }
    }
    printf("Step 2 returning: -1\n");
    return -1;
}

int start_sync(){
    bool go_on = false;
    if(sync_step1()){
        printf("Sync Step 1: failed\n");
    } else{
        printf("Sync Step 1: success\n");
        go_on = true;
    }

    if(go_on){
        if(sync_step2()){
            printf("Sync Step 2: failed\n");
            go_on = false;
        } else{
            printf("Sync Step 2: success\n");
        }
    }

    return 0;
}

std::string ready_response(const std::string& cmd){
    char buffer[BUFFER_SIZE] = {0};
    if (!strcmp(SYNC_START, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_START, my_add.data());
    } else if (!strcmp(SYNC_ACK_OK, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_ACK_OK, my_add.data(), read_recent_line_number(file_path).data());
    } else if (!strcmp(SYNC_ACK_FAIL, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_ACK_FAIL, my_add.data());
    }
    return buffer;
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