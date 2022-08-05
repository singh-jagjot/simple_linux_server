//
// Created by jj on 02/08/22.
//

#ifndef BBSERVER2_PEER_NOTIFY_H
#define BBSERVER2_PEER_NOTIFY_H

#include <string>
#include <set>

extern long send_to_peer(std::string ip, int port, std::string msg);
extern long send_to_all(std::set<std::string> &peers, const std::string& msg);

#endif //BBSERVER2_PEER_NOTIFY_H
