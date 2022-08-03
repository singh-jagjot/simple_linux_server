//
// Created by jj on 23/07/22.
//

#ifndef BBSERVER2_PEER_HANDLER_H
#define BBSERVER2_PEER_HANDLER_H

#include <string>
#include <netinet/in.h>

#define SYNC_START "SYNC GET READY"
#define SYNC_ACK_OK "GET READY OK"
#define SYNC_ACK_FAIL "GET READY FAILED"
#define SYNC_ABORT "SYNC ABORT"
#define SYNC_COMMIT "DO COMMIT "
#define SYNC_COMMIT_FAIL "COMMIT FAILED"
#define SYNC_COMMIT_OK "COMMIT OK"
#define SYNC_FAILED "SYNC FAILED"
#define SYNC_COMPLETED "SYNC DONE"
#define SYNC_TIMEOUT 60s

extern int handle_peer(int peer_fd, std::vector<std::string> &peers_add, std::vector<int> &peers_port);

extern long send_to_peer(std::string add, int port, std::string msg);

extern void master_set();

extern void master_unset();

extern int write_operation(std::string &msg);
void wait(int duration)

#endif //BBSERVER2_PEER_HANDLER_H
