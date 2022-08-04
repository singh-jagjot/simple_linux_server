//
// Created by jj on 02/08/22.
//

#ifndef BBSERVER2_CONNECT_PEER_H
#define BBSERVER2_CONNECT_PEER_H

#include <netinet/in.h>
#include <map>
#include <string>
#include <vector>

extern void create_socket_peer(std::map<std::string, std::string> &args);

//void accept_peers(int socket_fd, sockaddr_in &socket_add, int socket_add_len, std::vector<std::string> &peers_add,
//                  std::vector<int> &peers_port, std::string &iadd, std::string &file);
[[noreturn]] void accept_peers(int socket_fd, sockaddr_in &socket_add, int socket_add_len, std::map<std::string, std::string> &args);
extern void master_socket_peer_close();

#endif //BBSERVER2_CONNECT_PEER_H
