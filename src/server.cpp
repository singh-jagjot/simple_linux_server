//
// Created by jj on 22/07/22.
//
#include <thread>
#include <cstring>
#include "../include/common.h"
#include "../include/client_handler.h"
#include "../include/connect_client.h"
#include "../include/connect_peer.h"
#include "../include/server.h"
#include "../include/string_helper.h"

//Main method that handles teh server control.
int run_server(std::map<std::string, std::string> &args) {
    if(!args[PEERS].empty()) {
        if(!strcmp(args[DEBUG].data(), "1")){
            printf("Peers present\n");
        }
        //  Creating socket accept_clients for the peers_set
        std::thread t1(create_socket_peer, std::ref(args));
        std::thread t2(create_socket_client, std::ref(args));
        t1.join();
        t2.join();
    } else{
        create_socket_client(args);
    }

    return 0;
}