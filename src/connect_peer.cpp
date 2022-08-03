//
// Created by jj on 02/08/22.
//

#include <arpa/inet.h>
#include <thread>
#include <vector>
#include "../include/common.h"
#include "../include/connect_peer.h"
#include "../include/peer_handler.h"
#include "../include/string_helper.h"

int master_socket_peer;

void create_socket_peer(std::map<std::string, std::string> &args) {
    std::vector<std::string> peers = string_split(args[PEERS], ' ');
    std::vector<std::string> peers_add;
    std::vector<int> peers_port;

    for(auto &peer: peers){
        auto ipv4_add = string_split(peer, ':');
        if(ipv4_add.size() != 2){
            printf("Wrong IP address: %s\n", peer.data());
            exit(EXIT_FAILURE);
        }
        peers_add.push_back(ipv4_add[0]);
        peers_port.push_back(std::stoi(ipv4_add[1]));
    }

//    while (1){
//        send_to_peer(peers_add[0], peers_port[0], "This is a peer!\n");
//        sleep(5);
//    }

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
    printf("Binding to port %i successful.\n", connection_port);

    //Start listening the socket and hold max_threads connections
    if (listen(master_socket_peer, peers.size()) < 0) {
        printf("Listening on the socket failed. Error No: %i\n", errno);
        exit(EXIT_FAILURE);
    }
    printf("Listening on the socket successful.\n");

    std::vector<std::thread> threads;
    for (int i = 0; i < peers.size(); ++i) {
        threads.emplace_back(accept_peers, master_socket_peer, std::ref(socket_add), socket_add_len, std::ref(peers_add), std::ref(peers_port));
    }

    for (auto &t: threads) {
        t.join();
    }
}

void accept_peers(const int socket_fd, sockaddr_in &socket_add, const int socket_add_len, std::vector<std::string> &peers_add, std::vector<int> &peers_port) {
    //  Grabbing a connection from the listening queue
    while (true) {
        int peer_fd = accept(socket_fd, (struct sockaddr *) &socket_add, (socklen_t *) &socket_add_len);
        if (peer_fd < 0) {
            printf("Failed to accept peer. Error No: %i\n", errno);
            sleep(1);
            continue;
        }
//        auto add = inet_ntoa(socket_add.sin_addr);
//        auto port = ntohs(socket_add.sin_port);
//        printf("Peer connected. Address: %s:%u\n", add, port);
        printf("Peer connected\n");

        handle_peer(peer_fd, peers_add, peers_port);

        //closing the connected socket
        shutdown(peer_fd, SHUT_RDWR);
        close(peer_fd);
//        printf("Peer disconnected. Address: %s:%i\n", add, port);
        printf("Peer disconnected\n");
    }
}

void master_socket_peer_close() {
    shutdown(master_socket_peer, SHUT_RDWR);
    close(master_socket_peer);
    printf("Peer master socket closed.\n");
}