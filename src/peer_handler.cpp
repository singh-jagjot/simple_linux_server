//
// Created by jj on 23/07/22.
//
#include <vector>
#include <cstring>
#include <poll.h>
#include <set>
#include <stdexcept>
#include <unistd.h>
#include "../include/common.h"
#include "../include/peer_handler.h"
#include "../include/string_helper.h"
#include "../include/db_file_util.h"
#include "../include/peer_notify.h"

static bool is_master = false;
static bool in_progress = false;
static bool changes_committed = false;
int pfd;
sockaddr_in pfd_socket_add;
static std::set<std::string> peers_set;
static std::set<std::string> peers_with_ack;
static std::set<std::string> peers_with_commit;
static int msg_no = 0;
static std::string commit_msg;
static std::string client_name;
static bool is_write_msg;
std::string my_add;
static std::string master_add;
static std::string master_ip;
static int master_port;
static int client_fd;
std::string file_path;
static bool is_debug = false;

void set_peers(std::string &p){
    if(!peers_set.empty()){
        return;
    }

    for (auto &peer: string_split(p, ' ')) {
        peers_set.insert(peer);
    }

//    std::vector<std::string> peers_ip;
//    std::vector<int> peers_port;
//    for (auto &peer: p) {
//        auto ipv4_add = string_split(peer, ':');
//        if (ipv4_add.size() != 2) {
//            printf("Wrong IP address: %s\n", peer.data());
//            exit(EXIT_FAILURE);
//        }
//        peers_ip.push_back(ipv4_add[0]);
//        peers_port.push_back(std::stoi(ipv4_add[1]));
//        printf("Peer added: %s:%s\n", ipv4_add[0].data(),ipv4_add[1].data());
//    }
//
//    peers_ip_vec = peers_ip;
//    peers_port_vec = peers_port;
}

//long get_peer_count(){
//    return peers_set.size();
//}

void set_my_add(std::string &iadd){
    my_add = iadd;
//    auto ip_vec = string_split(iadd, ':');
//    my_ip = ip_vec[0];
//    my_port = ip_vec[1];
}

//Method to handle Peer related Protocol
//This uses the macros defined in
//peer_handler.h header file
//to compare the user input and perform
//the related function.
int handle_peer(const int peer_fd, std::map<std::string, std::string> &args){

    if(!strcmp(args[DEBUG].data(), "1")){
        is_debug = true;
    }

    pfd = peer_fd;
    if(is_debug){
        printf("Socket: %i\n", pfd);
    }
    char read_buffer[BUFFER_SIZE] = {0};
    char response_buffer[BUFFER_SIZE] = {0};
    file_path = args[BBFILE];
    std::vector<std::string> peer_msgs;

    while (true) {
        bzero(response_buffer, BUFFER_SIZE);
        bzero(read_buffer, BUFFER_SIZE);
        long bytes_read = read(peer_fd, read_buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            printf("Error while reading the read_buffer. Error No: %i\n", errno);
            exit(EXIT_FAILURE);
        }
        if (bytes_read == 0){
            if(is_debug){
                printf("Peer exited abruptly\n");
            }
            break;
        }
        if(is_debug){
            printf("Bytes read: %li, Message: %s", bytes_read, read_buffer);
        }

        if(string_startswith(read_buffer, SYNC_START)) {
            try {
                in_progress = true;
                master_add = string_split(read_buffer, '|')[1];
                auto v = string_split(master_add, ':');
                master_ip = v[0];
                master_port = std::stoi(v[1]);
                read_lock_acquire();
                write_lock_acquire();
                if(is_debug){
                    printf("Peer: Read and Write lock acquired\n");
                }
                if(send_to_peer(master_ip, master_port, ready_response(SYNC_ACK_OK)) == -1){
                    throw std::runtime_error("Peer: Failed to send acknowledgement\n");
                } else{
                    if(is_debug){
                        printf("Peer: Sent %s signal to the master\n", SYNC_ACK_OK);
                    }
                }
            } catch (...) {
                if(is_debug){
                    printf("Peer: Failed to send %s signal to the master\n", SYNC_ACK_OK);
                }
            }
        } else if (string_startswith(read_buffer, SYNC_ACK_FAIL)){
            if(is_debug){
                printf("Master: Received %s signal from peer: %s, Aborting\n", SYNC_ACK_FAIL, string_split(read_buffer, '|')[1].data());
            }
            if(send_to_all(peers_set, ready_response(SYNC_ABORT)) != 0){
                if(is_debug){
                    printf("Master: Some error occurred while sending %s signal\n", SYNC_ABORT);
                }
            } else {
                if(is_debug){
                    printf("Master: Aborting Success\n");
                }
            }
            write_to_client_console(false);
            set_to_default();
        } else if(string_startswith(read_buffer, SYNC_ACK_OK)){
            auto vec = string_split(read_buffer, '|');
            auto peer_ipv4 = vec[1];
            auto peer_resent_msg_no = std::stoi(vec[2]);
            if(is_debug){
                printf("Master: Received %s signal from peer %s\n", SYNC_ACK_OK, peer_ipv4.data());
            }
            if(peers_set.contains(peer_ipv4)){
                peers_with_ack.insert(peer_ipv4);
                msg_no = msg_no > peer_resent_msg_no ? msg_no : peer_resent_msg_no;
            }
            if(peers_with_ack.size() == peers_set.size()){
                if(is_debug){
                    printf("Master: Received %s signal from all the peers_set. Sending %s signal\n", SYNC_ACK_OK, SYNC_COMMIT);
                }
                if(send_to_all(peers_set, ready_response(SYNC_COMMIT)) != 0){
                    if(is_debug){
                        printf("Master: Some error occurred while sending %s signal\n", SYNC_COMMIT);
                    }
                } else {
                    if(is_debug){
                        printf("Master: Committing Success\n");
                    }
                }
            }
        } else if(string_startswith(read_buffer, SYNC_ABORT)){
            if(is_debug){
                printf("Peer: Received %s signal. Aborting and releasing locks..\n", SYNC_ABORT);
            }
            if(changes_committed){
                if(is_write_msg){
                    if(undo_synced_write_message(args[BBFILE], is_debug) != 0){
                        if(is_debug){
                            printf("Peer: Some error occurred wile undoing\n");
                        }
                    } else{
                        if(is_debug){
                            printf("Peer: Undoing Success\n");
                        }
                    }
                } else {
                    if(undo_synced_replace_message(args[BBFILE], is_debug) != 0){
                        if(is_debug){
                            printf("Peer: Some error occurred wile undoing\n");
                        }
                    } else{
                        if(is_debug){
                            printf("Peer: Undoing Success\n");
                        }
                    }
                }
            }
            set_to_default();
        } else if(string_startswith(read_buffer, SYNC_COMMIT)){
            auto msg_vec = string_file_msg_split(read_buffer, '|', 2);
            try {
                if(!strcmp(msg_vec[1].data(), "WRITE")){
                    is_write_msg = true;
                    if(is_debug){
                        printf("Peer: %s signal received. Writing %s to file\n", SYNC_COMMIT, msg_vec[2].data());
                    }
                    if(write_synced_message(args[BBFILE], msg_vec[2].data(), is_debug) != 0){
                        throw std::runtime_error("Peer: Failed to write synced message\n");
                    } else {
                        if(send_to_peer(master_ip, master_port, ready_response(SYNC_COMMIT_OK)) != -1){
                            if(is_debug){
                                printf("Peer: Sent %s signal to master\n", SYNC_COMMIT_OK);
                            }
                        } else{
                            if(is_debug){
                                printf("Peer: Sending %s signal to master failed\n", SYNC_COMMIT_OK);
                            }
                        }
                        changes_committed = true;
                    }
                } else {
                    if(is_debug){
                        printf("Peer: %s signal received. Replacing %s to file\n", SYNC_COMMIT, msg_vec[2].data());
                    }
                        if(replace_synced_message(args[BBFILE], msg_vec[2].data(), is_debug) != 0){
                            throw std::runtime_error("Peer: Failed to replace synced message\n");
                        } else {
                            if(send_to_peer(master_ip, master_port, ready_response(SYNC_COMMIT_OK)) != -1){
                                if(is_debug){
                                    printf("Peer: Sent %s signal to master\n", SYNC_COMMIT_OK);
                                }
                            } else{
                                if(is_debug){
                                    printf("Peer: Sending %s signal to master failed\n", SYNC_COMMIT_OK);
                                }
                            }
                            changes_committed = true;
                        }
                }
            } catch (...){
                if(is_debug){
                    printf("Peer: Some error occurred while committing the msg: %s\n", msg_vec[2].data());
                }
                if(send_to_peer(master_ip, master_port, ready_response(SYNC_COMMIT_FAIL))  == -1){
                    if(is_debug){
                        printf("Peer: Sending %s signal to the master failed\n", SYNC_COMMIT_FAIL);
                    }
                } else{
                    if(is_debug){
                        printf("Peer: Sent %s signal to the master\n", SYNC_COMMIT_FAIL);
                    }
                }
                set_to_default();
            }
        } else if(string_startswith(read_buffer, SYNC_COMMIT_FAIL)){
            if(is_debug){
                printf("Master: %s signal received from peer %s\n", SYNC_COMMIT_FAIL, string_split(read_buffer, '|')[1].data());
            }
            if(send_to_all(peers_set, ready_response(SYNC_FAILED)) != 0){
                if(is_debug){
                    printf("Master: Some error occurred while sending %s signal\n", SYNC_FAILED);
                }
            } else{
                if(is_debug){
                    printf("Master: Sent %s signal to all peers_set", SYNC_FAILED);
                }
            }
            set_to_default();
            write_to_client_console(false);
        } else if(string_startswith(read_buffer, SYNC_COMMIT_OK)){
            auto peer_ipv4 = string_split(read_buffer, '|')[1];
            if(is_debug){
                printf("Master: Received %s signal form the peer %s\n", SYNC_COMMIT_OK, peer_ipv4.data());
            }
            if(peers_set.contains(peer_ipv4)) {
                peers_with_commit.insert(peer_ipv4);
            }
            if(peers_with_commit.size() == peers_set.size()){
                if(is_debug){
                    printf("Master: Received %s signal from all the peers_set\n", SYNC_COMMIT_OK);
                }
                try {
                    if(is_write_msg){
                        if(is_debug){
                            printf("Master: Writing %s to file\n", commit_msg.data());
                        }
                        if(write_synced_message(args[BBFILE], commit_msg, is_debug) != 0){
                            throw std::runtime_error("Master: Failed to write synced message\n");
                        } else {
                            if(is_debug){
                                printf("Master: Committing to file completed\n");
                            }
                            if(send_to_all(peers_set, ready_response(SYNC_COMPLETED)) != 0){
                                if(is_debug){
                                    printf("Master: Some error occurred while sending %s signal\n", SYNC_COMPLETED);
                                }
                            } else {
                                if(is_debug){
                                    printf("Master: Sent %s signal to all peers_set\n", SYNC_COMPLETED);
                                }
                            }
                        }
                    } else{
                        if(is_debug){
                            printf("Master: Replacing %s to file\n", commit_msg.data());
                        }
                        if(replace_synced_message(args[BBFILE], commit_msg, is_debug) != 0){
                            throw std::runtime_error("Master: Failed to replace synced message\n");
                        } else {
                            if(is_debug){
                                printf("Master: Committing to file completed\n");
                            }
                            if(send_to_all(peers_set, ready_response(SYNC_COMPLETED)) != 0){
                                if(is_debug){
                                    printf("Master: Some error occurred while sending %s signal\n", SYNC_COMPLETED);
                                }
                            } else {
                                if(is_debug){
                                    printf("Master: Sent %s signal to all peers_set\n", SYNC_COMPLETED);
                                }
                            }
                        }
                    }
                    write_to_client_console(true);
                } catch (...) {
                    if(is_debug){
                        printf("Master: Some error occurred while committing the msg: %s\n", commit_msg.data());
                    }
                    if(send_to_all(peers_set, ready_response(SYNC_FAILED)) != 0){
                        if(is_debug){
                            printf("Master: Some error occurred while sending %s signal to peers_set\n", SYNC_FAILED);
                        }
                    } else {
                        if(is_debug){
                            printf("Master: Sent %s signal to peers_set\n", SYNC_FAILED);
                        }
                    }
                    write_to_client_console(false);
                }
                set_to_default();
            }
        } else if(string_startswith(read_buffer, SYNC_FAILED)){
            if(is_debug){
                printf("Peer: Received %s signal from master. Undoing committed changes\n", SYNC_FAILED);
            }
            if(changes_committed){
                if(is_write_msg){
                    if(undo_synced_write_message(args[BBFILE], is_debug) != 0){
                        if(is_debug){
                            printf("Peer: Some error occurred wile undoing\n");
                        }
                    } else{
                        if(is_debug){
                            printf("Peer: Undoing Success\n");
                        }
                    }
                } else {
                    if(undo_synced_replace_message(args[BBFILE], is_debug) != 0){
                        if(is_debug){
                            printf("Peer: Some error occurred wile undoing\n");
                        }
                    } else{
                        if(is_debug){
                            printf("Peer: Undoing Success\n");
                        }
                    }
                }
            }
            set_to_default();
        } else if(string_startswith(read_buffer, SYNC_COMPLETED)){
            if(is_debug){
                printf("%s: lock released\n", SYNC_COMPLETED);
            }
            set_to_default();
        } else if(string_startswith(read_buffer, QUIT)){
            if(is_debug){
                printf("Peer exiting\n");
            }
            break;
        } else {
            sprintf(response_buffer,"Invalid cmd: %s\n", read_buffer);
        }
//        write(peer_fd, response_buffer, BUFFER_SIZE);
    }
    return 1;
}

//void master_set(){
//    is_master = true;
//}
//
//void master_unset(){
//    is_master = false;
//}

//Step 1: Notify for sync
//int sync_step1(){
//    if(is_debug){
//        printf("Peers set size: %zu\n", peers_set.size());
//    }
//    for(auto &peer: peers_set){
//        auto vec = string_split(peer, ':');
//        auto add = vec[0];
//        auto port = std::stoi(vec[1]);
//        if(send_to_peer(add, port, ready_response(SYNC_START)) == -1){
//            return -1;
//        }
//    }
//    printf("Step 1 returned 0\n");
//    return 0;
//}

//Step 2: Wait for acknowledgement
//int sync_step2(){
//    struct pollfd p[1];
//    p[0].fd = pfd;
//    p[0].events = POLLIN;
//    printf("Monitored FD: %i\n", pfd);
//    char read_buffer[BUFFER_SIZE] = {0};
//    int events_num = poll(p, 1, 10000);
//    if(events_num == 0){
//        printf("Wait timed out for acknowledgments\n");
//    } else {
//        int pollin_happened = p[0].revents & POLLIN;
//        if(pollin_happened){
//            long bytes_read = read(p[0].fd, read_buffer, BUFFER_SIZE);
//            if (bytes_read < 0) {
//                printf("Error while reading the read_buffer. Error No: %i\n", errno);
//                return -1;
//            }
//            printf("Polled: %s\n", read_buffer);
//            printf("Step 2 returning: 0\n");
//            return 0;
//        } else {
//            printf("System error occurred: %d\n", p[0].revents);
//        }
//    }
//    printf("Step 2 returning: -1\n");
//    return -1;
//}

int start_sync(int port, std::string &client, std::string msg, const int fd){
    is_write_msg = string_startswith(msg, CMD_WRITE);
    client_fd = fd;
    commit_msg = string_file_msg_split(msg, ' ', 1)[1];
    client_name = client;
    is_master = true;
//    bool go_on = false;
    write_lock_acquire();
    read_lock_acquire();
    if(is_debug){
        printf("Peers set size: %zu\n", peers_set.size());
    }
    for(auto &peer: peers_set){
        auto vec = string_split(peer, ':');
        auto p_add = vec[0];
        auto p_port = std::stoi(vec[1]);
        if(send_to_peer(p_add, p_port, ready_response(SYNC_START)) == -1){
            return -1;
        }
    }
//    if(sync_step1()){
//        printf("Sync Step 1: failed\n");
//    } else{
//        printf("Sync Step 1: success\n");
//        go_on = true;
//    }
//Todo Remove deadlock here(if one of the peers exit abruptly)
//Todo Implement timeout

//    if(go_on && !pfd){
//        try {
//            //    Creating a IPv4 and TCP socket
//            int receiving_socket = socket(AF_INET, SOCK_STREAM, 0);
//            if (receiving_socket == -1) {
//                printf("Failed to create TCP socket. Error No: %i\n", errno);
//                exit(EXIT_FAILURE);
//            }
//
//            // Setting option to reuse the socket
//            int reuse = 1;
//            setsockopt(receiving_socket, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &reuse, sizeof(reuse));
//
//            sockaddr_in socket_add{};
//            socket_add.sin_family = AF_INET;
//            socket_add.sin_addr.s_addr = INADDR_ANY;
//            socket_add.sin_port = htons(port);
//            int socket_add_len = sizeof(socket_add);
//
//            if (bind(receiving_socket, (struct sockaddr *) &socket_add, socket_add_len) < 0) {
//                printf("Binding to port %i failed. Error No: %i\n", port, errno);
//                exit(EXIT_FAILURE);
//            }
//            printf("Binding to port %i successful.\n", port);
//
//            //Start listening the socket and hold max connection requests
//            if (listen(receiving_socket, 1) < 0) {
//                printf("Listening(preparing to listen) on the socket failed. Error No: %i\n", errno);
//                exit(EXIT_FAILURE);
//            }
//            printf("Listening(preparing to listen) on the receiving socket %i successful.\n", receiving_socket);
//            pfd = accept(receiving_socket, (struct sockaddr *) &socket_add, (socklen_t *) &socket_add_len);
//        } catch (...) {
//            return -1;
//        }
//    }
//
//    if(go_on){
//        if(sync_step2()){
//            printf("Sync Step 2: failed\n");
//            go_on = false;
//        } else{
//            printf("Sync Step 2: success\n");
//        }
//    }

//    shutdown(pfd, SHUT_RDWR);
//    close(pfd);
//    is_master = false;
    return 0;
}

//Method to ready the peer response based on the signal received.
std::string ready_response(const std::string& cmd){
    char buffer[BUFFER_SIZE] = {0};
    if (!strcmp(SYNC_START, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_START, my_add.data());
    } else if (!strcmp(SYNC_ACK_OK, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_ACK_OK, my_add.data(), read_recent_line_number(file_path).data());
    } else if (!strcmp(SYNC_ACK_FAIL, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_ACK_FAIL, my_add.data());
    } else if (!strcmp(SYNC_ABORT, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_ABORT);
    } else if (!strcmp(SYNC_COMMIT, cmd.data())){
        if(is_write_msg){
            int here_resent_msg_no = std::stoi(read_recent_line_number(file_path));
            msg_no = msg_no > here_resent_msg_no ? msg_no : here_resent_msg_no;
            commit_msg = std::to_string(++msg_no) + "/" + client_name + "/" + commit_msg;
            sprintf(buffer, FORMAT_SYNC_COMMIT, "WRITE", commit_msg.data());
        } else {
            commit_msg = commit_msg + "/" + client_name;
            sprintf(buffer, FORMAT_SYNC_COMMIT, "REPLACE", commit_msg.data());
        }
    } else if (!strcmp(SYNC_COMMIT_OK, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_COMMIT_OK, my_add.data());
    } else if (!strcmp(SYNC_COMMIT_FAIL, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_COMMIT_FAIL, my_add.data());
    } else if (!strcmp(SYNC_FAILED, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_FAILED);
    } else if (!strcmp(SYNC_COMPLETED, cmd.data())){
        sprintf(buffer, FORMAT_SYNC_COMPLETED);
    }
    return buffer;
}

//Method to write on the client console based on whether it's ERROR or SUCCESS message
void write_to_client_console(bool no_error) {
    char temp_buffer[BUFFER_SIZE] = {0};
    if(no_error){
        sprintf(temp_buffer, "%s %i\n", RES_MESSAGE_WROTE, msg_no);
    } else {
//        if(is_write){
//            sprintf(temp_buffer,"%s Server error\n", RES_MESSAGE_WROTE_ERR);
//        } else{
//            sprintf(temp_buffer, "%s %s\n", RES_REPLACE_MESSAGE_NE);
//        }
        sprintf(temp_buffer,"%s Server error\n", RES_MESSAGE_WROTE_ERR);
    }
    write(client_fd, temp_buffer, BUFFER_SIZE);
}

//Method to set all the values to the default.
void set_to_default() {
    peers_with_ack.clear();
    peers_with_commit.clear();
    msg_no = 0;
    commit_msg.clear();
    client_name.clear();
    is_write_msg = false;
    changes_committed = false;
    in_progress = false;
    master_add.clear();
    master_ip.clear();
    master_port = 0;
    read_lock_release();
    write_lock_release();
}