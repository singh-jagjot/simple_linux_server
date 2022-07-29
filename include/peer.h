//
// Created by jj on 23/07/22.
//

#ifndef BBSERVER2_PEER_H
#define BBSERVER2_PEER_H

#include <string>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define SYNC_START "SYNC GET READY"
#define SYNC_ACK_OK "GET READY OK"
#define SYNC_ACK_FAIL "GET READY FAILED"
#define SYNC_ABORT "SYNC ABORT"
#define SYNC_COMMIT "DO COMMIT "
#define SYNC_COMMIT_FAIL "COMMIT FAILED"
#define SYNC_COMMIT_OK "COMMIT OK"
#define SYNC_FAILED "SYNC FAILED"
#define SYNC_COMPLETED "SYNC DONE"

extern int peer_socket_fd;
extern bool is_master;

extern int handle_peer(const int &socket_fd, const std::string &file_path, sockaddr_in &socket_add);

extern long send_to_peer(std::string msg);

extern void master_set();

extern void master_unset();

#endif //BBSERVER2_PEER_H
