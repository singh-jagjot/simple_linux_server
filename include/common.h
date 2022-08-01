//
// Created by jj on 22/07/22.
//

#ifndef BBSERVER2_COMMON_H
#define BBSERVER2_COMMON_H


#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>


extern void create_socket_connection(std::string address, std::string is_daemon, int port, int max_threads, const std::string &file_path,
                              int (*handle)(const int &socket_fd, const std::string &file_path, sockaddr_in &socket_add));

extern void connection(std::string is_daemon, int &socket_fd, sockaddr_in &socket_add, int &socket_add_len, const std::string &file_path,
                       int (*handle)(const int &socket_fd, const std::string &file_path, sockaddr_in &socket_add));

extern void master_socket_close();

#endif //BBSERVER2_COMMON_H
