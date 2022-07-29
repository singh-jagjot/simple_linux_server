//
// Created by jj on 22/07/22.
//

#ifndef BBSERVER2_SERVER_H
#define BBSERVER2_SERVER_H

#include <string>
#include <map>

#define THMAX "THMAX"
#define BBPORT "BBPORT"
#define SYNCPORT "SYNCPORT"
#define BBFILE "BBFILE"
#define PEERS "PEERS"
#define DAEMON "DAEMON"
#define DEBUG "DEBUG"
#define CONFIG_FILE "CONFIG_FILE"

extern int run_server(std::map<std::string, std::string> &args);

#endif //BBSERVER2_SERVER_H
