//
// Created by jj on 23/07/22.
//

#ifndef BBSERVER2_PEER_HANDLER_H
#define BBSERVER2_PEER_HANDLER_H

#include <string>
#include <netinet/in.h>
#include <map>

#define SYNC_START "SYNC GET READY"
#define SYNC_ACK_OK "GET READY OK"
#define SYNC_ACK_FAIL "GET READY FAILED"
#define SYNC_ABORT "SYNC ABORT"
#define SYNC_COMMIT "DO COMMIT"
#define SYNC_COMMIT_FAIL "COMMIT FAILED"
#define SYNC_COMMIT_OK "COMMIT OK"
#define SYNC_FAILED "SYNC FAILED"
#define SYNC_COMPLETED "SYNC DONE"

#define FORMAT_SYNC_START "SYNC GET READY|%s"          //msg|master_IPv4
#define FORMAT_SYNC_ACK_OK "GET READY OK|%s|%s"        //msg|peer_IPv4|latest_msg_no
#define FORMAT_SYNC_ACK_FAIL "GET READY FAILED|%s"     //msg|peer_IPv4
#define FORMAT_SYNC_ABORT "SYNC ABORT"
#define FORMAT_SYNC_COMMIT "DO COMMIT|%s"
#define FORMAT_SYNC_COMMIT_FAIL "COMMIT FAILED|%s"
#define FORMAT_SYNC_COMMIT_OK "COMMIT OK|%s"
#define FORMAT_SYNC_FAILED "SYNC FAILED|%s"
#define FORMAT_SYNC_COMPLETED "SYNC DONE|%s"

#define SYNC_TIMEOUT 60s

//extern int
//handle_peer(int peer_fd, std::vector<std::string> &peers_add, std::vector<int> &peers_port, std::string &iadd,
//            std::string &file);

extern int handle_peer(int peer_fd, std::map<std::string, std::string> &args);

extern void set_peers(std::string &peers);

extern void set_my_add(std::string &iadd);

extern long get_peer_count();

extern void master_set();

extern void master_unset();

extern int write_operation(std::string &msg);

void wait(int duration);

int start_sync();

std::string ready_response(const std::string& cmd);

#endif //BBSERVER2_PEER_HANDLER_H
