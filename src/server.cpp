//
// Created by jj on 22/07/22.
//

#include "../include/common.h"
#include "../include/client_handler.h"
#include "../include/connect_client.h"
#include "../include/connect_peer.h"
//#include "../include/peer.h"
#include "../include/server.h"
#include "../include/string_helper.h"

int run_server(std::map<std::string, std::string> &args) {
    //  Creating socket connection for the peers
//    auto peers = string_split(args[PEERS], ' ');
//    create_socket_connection("",args[DAEMON], std::stoi(args[SYNCPORT]), peers.size(), args[BBFILE], &handle_peer);
//    for (auto &add: peers) {
//        auto address = string_split(add, ':');
//        create_socket_connection(address[0], std::stoi(args[SYNCPORT]), peers.size(), args[BBFILE], &handle_peer);
//    }
    create_socket_peer(args);
    //  Creating socket connection for the clients
//    create_socket_connection("", args[DAEMON], std::stoi(args[BBPORT]), std::stoi(args[THMAX]), args[BBFILE], &handle_client);
    return 0;
}