//
// Created by jj on 23/07/22.
//

#ifndef BBSERVER2_PEER_H
#define BBSERVER2_PEER_H

#include <string>
#define BUFFER_SIZE 1024

extern int peer_socket_fd;
extern int handle_peer(const int& socket_fd, const std::string& file_path);
extern long send_to_peer(std::string msg);
#endif //BBSERVER2_PEER_H
