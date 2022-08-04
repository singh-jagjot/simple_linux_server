//
// Created by jj on 22/07/22.
//

#ifndef BBSERVER2_CONNECT_CLIENT_H
#define BBSERVER2_CONNECT_CLIENT_H


#include <arpa/inet.h>
#include <map>
#include <netinet/in.h>
#include <string>


//extern void create_socket_connection(std::string address, std::string is_daemon, int port, int max_threads, const std::string &file_path,
//                              int (*handle)(const int &master_socket_client, const std::string &file_path, sockaddr_in &socket_add));

extern void create_socket_client(std::map<std::string, std::string> &args);
//extern void accept_clients(std::string is_daemon, int &socket_fd, sockaddr_in &socket_add, int &socket_add_len, const std::string &file_path,
//                           int (*handle)(const int &socket_fd, const std::string &file_path, sockaddr_in &socket_add));
[[noreturn]] extern void accept_clients(int &socket_fd, sockaddr_in &socket_add, int &socket_add_len, std::map<std::string, std::string> &args);
extern void master_socket_close();

#endif //BBSERVER2_CONNECT_CLIENT_H
