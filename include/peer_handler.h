//
// Created by jj on 23/07/22.
//

#ifndef BBSERVER2_PEER_HANDLER_H
#define BBSERVER2_PEER_HANDLER_H

#include <string>
#include <netinet/in.h>
#include <map>

#define SYNC_START "01 SYNC GET READY"
#define SYNC_ACK_OK "02 SYNC GET READY OK"
#define SYNC_ACK_FAIL "03 SYNC GET READY FAILED"
#define SYNC_ABORT "04 SYNC ABORT"
#define SYNC_COMMIT "05 SYNC DO COMMIT"
#define SYNC_COMMIT_FAIL "06 SYNC COMMIT FAILED"
#define SYNC_COMMIT_OK "07 SYNC COMMIT OK"
#define SYNC_FAILED "08 SYNC FAILED"
#define SYNC_COMPLETED "09 SYNC COMPLETED"

#define FORMAT_SYNC_START "01 SYNC GET READY|%s"               //signal|master_IPv4
#define FORMAT_SYNC_ACK_OK "02 SYNC GET READY OK|%s|%s"        //signal|peer_IPv4|latest_msg_no
#define FORMAT_SYNC_ACK_FAIL "03 SYNC GET READY FAILED|%s"     //signal|peer_IPv4
#define FORMAT_SYNC_ABORT "04 SYNC ABORT"                      //signal
#define FORMAT_SYNC_COMMIT "05 SYNC DO COMMIT|%s|%s"           //signal|msg type|msg to commit
#define FORMAT_SYNC_COMMIT_FAIL "06 SYNC COMMIT FAILED|%s"     //signal|peer_IPv4
#define FORMAT_SYNC_COMMIT_OK "07 SYNC COMMIT OK|%s"           //signal|peer_IPv4
#define FORMAT_SYNC_FAILED "08 SYNC FAILED"                    //signal
#define FORMAT_SYNC_COMPLETED "09 SYNC COMPLETED"              //signal

#define CMD_WRITE "WRITE "
#define RES_MESSAGE_WROTE "3.0 WROTE"
#define RES_MESSAGE_WROTE_ERR "3.2 ERROR WRITE"
#define RES_REPLACE_MESSAGE_NE "3.1 UNKNOWN"
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

//extern int write_operation(std::string &msg);

extern void set_pfd_socket_add(sockaddr_in &socket_add);

extern void set_pfd(int fd);

void wait(int duration);

int start_sync(int port, std::string &client_name, std::string msg, int client_fd);

void write_to_client_console(bool no_error);

void set_to_default();

int sync_step2();

std::string ready_response(const std::string &cmd);

#endif //BBSERVER2_PEER_HANDLER_H
