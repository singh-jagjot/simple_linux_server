//
// Created by jj on 02/08/22.
//

#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include <vector>
#include <unistd.h>
#include "../include/common.h"
#include "../include/connect_peer.h"
#include "../include/peer_handler.h"
#include "../include/string_helper.h"

int master_socket_peer;
static bool is_debug = false;

//Method to creat the peer master socket to accept
//peers for communication.
void create_socket_peer(std::map<std::string, std::string> &args) {
    if(!strcmp(args[DEBUG].data(), "1")){
        is_debug = true;
    }
    set_peers(args[PEERS]);
    std::vector<std::string> peers = string_split(args[PEERS], ' ');

    //    Creating a IPv4 and TCP socket
    master_socket_peer = socket(AF_INET, SOCK_STREAM, 0);
    if (master_socket_peer == -1) {
        printf("Failed to create TCP socket. Error No: %i\n", errno);
        exit(EXIT_FAILURE);
    }

    // Setting option to reuse the socket
    int reuse = 1;
    setsockopt(master_socket_peer, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &reuse, sizeof(reuse));

    int connection_port = std::stoi(args[SYNCPORT]);
    sockaddr_in socket_add{};
    socket_add.sin_family = AF_INET;
    socket_add.sin_addr.s_addr = INADDR_ANY;
    socket_add.sin_port = htons(connection_port);
    int socket_add_len = sizeof(socket_add);

    if (bind(master_socket_peer, (struct sockaddr *) &socket_add, socket_add_len) < 0) {
        printf("Binding to port %i failed. Error No: %i\n", connection_port, errno);
        exit(EXIT_FAILURE);
    }

    if(is_debug){
        printf("Binding to port %i successful.\n", connection_port);
    }

    //Start listening the socket and hold max_threads connections
    if (listen(master_socket_peer, peers.size()) < 0) {
        printf("Listening on the socket failed. Error No: %i\n", errno);
        exit(EXIT_FAILURE);
    }

    if(is_debug){
        printf("Listening on the socket successful.\n");
    }
    std::vector<std::thread> threads;
    for (int i = 0; i < 1; ++i) {
//        threads.emplace_back(accept_peers, master_socket_peer, std::ref(socket_add), socket_add_len, std::ref(args));
        accept_peers(master_socket_peer, socket_add, socket_add_len, args);
    }
    for (auto &t: threads) {
        t.join();
    }
}

// To keep on accepting new peers.
[[noreturn]] void accept_peers(const int socket_fd, sockaddr_in &socket_add, const int socket_add_len, std::map<std::string, std::string> &args) {
    std::string iadd = "127.0.0.1:" + args[SYNCPORT];
    set_my_add(iadd);

    //  Grabbing a accept_clients from the listening queue
    while (true) {
        int peer_fd = accept(socket_fd, (struct sockaddr *) &socket_add, (socklen_t *) &socket_add_len);
        if (peer_fd < 0) {
            printf("Failed to accept peer. Error No: %i\n", errno);
            sleep(1); //To make sure program doesn't exit after receiving SIGHUP.
            continue;
        }
        if(is_debug){
            printf("Master Peer FD: %i\n", peer_fd);
            printf("Peer connected\n");
        }
//        auto add = inet_ntoa(socket_add.sin_addr);
//        auto port = ntohs(socket_add.sin_port);
//        printf("Peer connected. Address: %s:%u\n", add, port);
        handle_peer(peer_fd, args);

        //closing the connected socket
        shutdown(peer_fd, SHUT_RDWR);
        close(peer_fd);
//        printf("Peer disconnected. Address: %s:%i\n", add, port);
        if(is_debug){
            printf("Peer disconnected\n");
        }
    }
}

void master_socket_peer_close() {
//Closing the master_peer_socket
    if(is_debug){
        if(shutdown(master_socket_peer, SHUT_RD) == -1){
            printf("Error while shutting down the master_socket_peer\n");
        } else {
            printf("Success: Shutting down the master_socket_peer\n");
        }
        if(close(master_socket_peer) == -1){
            printf("Error while closing the master_socket_peer\n");
        } else{
            printf("Success: Closing the master_socket_peer\n");
        }
    }
}