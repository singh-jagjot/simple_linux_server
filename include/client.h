//
// Created by jj on 22/07/22.
//

#ifndef BBSERVER2_CLIENT_H
#define BBSERVER2_CLIENT_H

#include <string>
#define BUFFER_SIZE 1024

//Server Responses
#define RES_INITIAL_GREETING "0.0 Hello!Welcome to Bulletin Board Server!\n"
#define RES_USER_GREETING "1.0 HELLO"
#define RES_USER_ERR "1.2 ERROR USER"
#define RES_MESSAGE_READ "2.0 MESSAGE"
#define RES_MESSAGE_READ_NE "2.1 UNKNOWN"
#define RES_MESSAGE_READ_ERR "2.2 ERROR READ"
#define RES_MESSAGE_WROTE "3.0 WROTE"
#define RES_MESSAGE_WROTE_ERR "3.2 ERROR WRITE"
#define RES_REPLACE_MESSAGE_NE "3.1 UNKNOWN"
#define RES_QUIT "4.0 BYE"
#define RES_USAGE "\nUsage:\nUSER name\nREAD message-number\nWRITE message\nREPLACE message-number/message\nQUIT text\n"

//Server commands
#define CMD_USER "USER "
#define CMD_READ "READ "
#define CMD_WRITE "WRITE "
#define CMD_REPLACE "REPLACE "
#define CMD_QUIT "QUIT "

extern int handle_client(const int& socket_fd, const std::string& file_path);
#endif //BBSERVER2_CLIENT_H
