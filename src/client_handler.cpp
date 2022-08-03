//
// Created by jj on 22/07/22.
//

#include <unistd.h>
#include <vector>
#include <cstring>
#include "../include/common.h"
#include "../include/client_handler.h"
#include "../include/db_file_util.h"
//#include "../include/peer.h"
#include "../include/string_helper.h"

int handle_client(const int &socket_fd, const std::string &file_path, sockaddr_in &socket_add) {
    std::string client_name = "nobody";
    char read_buffer[BUFFER_SIZE] = {0};
    char response_buffer[BUFFER_SIZE] = {0};
    std::vector<std::string> client_msgs;
    write(socket_fd, RES_INITIAL_GREETING, sizeof(RES_INITIAL_GREETING));

    while (true) {
        bzero(response_buffer, BUFFER_SIZE);
        long bytes_read = read(socket_fd, read_buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            printf("Error while reading the read_buffer. Error No: %i", errno);
            exit(EXIT_FAILURE);
        }

        if (bytes_read == 0){
            printf("Client exited abruptly\n");
            break;
        }

        if (string_startswith(read_buffer, CMD_USER)) {
            try {
                client_msgs = string_file_msg_split(read_buffer, ' ', 1);
                std::string temp = client_msgs.at(1);
                if (string_contains(temp, "\\") or string_contains(temp, "/") or string_contains(temp, "\t") or
                    string_contains(temp, "\n") or string_contains(temp, "\r")) {
                    sprintf(response_buffer, "%s Invalid usage or characters!\n", RES_USER_ERR);
                } else {
                    client_name = string_strip(temp);
                    printf("Client: %s\n", client_name.data());
                    sprintf(response_buffer, "%s %s Welcome!\n", RES_USER_GREETING, client_name.data());
                }
            } catch (...) {
                sprintf(response_buffer, "%s Invalid usage or characters!\n", RES_USER_ERR);
            }
        } else if (string_startswith(read_buffer, CMD_READ)) {
            try {
                std::string res = read_message(file_path, read_buffer);
                if (res.empty()) {
                    sprintf(response_buffer, "%s %s Message number not exist!\n", RES_MESSAGE_READ_NE,
                            string_file_msg_split(read_buffer, ' ', 1)[1].data());
                } else {
                    auto v = string_file_msg_split(res, '/', 2);
                    sprintf(response_buffer, "%s %s %s/%s\n", RES_MESSAGE_READ, v[0].data(), v[1].data(), v[2].data());
                }
            } catch (...) {
                sprintf(response_buffer, "%s Server error!\n", RES_MESSAGE_READ_ERR);
            }
        } else if (string_startswith(read_buffer, CMD_WRITE)) {
            try {
                int res = write_message(file_path, client_name, read_buffer);
                if (res == -1) {
                    sprintf(response_buffer, "%s Server error\n", RES_MESSAGE_WROTE_ERR);
                } else {
                    sprintf(response_buffer, "%s %i\n", RES_MESSAGE_WROTE, res);
                }
            } catch (...) {
                sprintf(response_buffer, "%s Server error\n", RES_MESSAGE_WROTE_ERR);
            }
        } else if (string_startswith(read_buffer, CMD_REPLACE)) {
            try {
                int res = replace_message(file_path, client_name, read_buffer);
                if (res != -1) {
                    sprintf(response_buffer, "%s %i\n", RES_MESSAGE_WROTE, res);
                } else {
                    sprintf(response_buffer, "%s %s\n", RES_REPLACE_MESSAGE_NE,
                            string_file_msg_split(read_buffer, ' ', 1)[1].data());
                }
            } catch (...) {
                sprintf(response_buffer, "%s Server error\n", RES_MESSAGE_WROTE_ERR);
            }
        } else if (string_startswith(read_buffer, CMD_QUIT)) {
            sprintf(response_buffer, "%s %s!\n", RES_QUIT, client_name.data());
            printf("Bytes read: %li, Message: %s", bytes_read, read_buffer);
            break;
        } else {
            sprintf(response_buffer, "%s%s", "Invalid cmd!", RES_USAGE);
            printf("%s", response_buffer);
        }
        write(socket_fd, response_buffer, BUFFER_SIZE);
        printf("Bytes read: %li, Message: %s", bytes_read, read_buffer);
        bzero(read_buffer, BUFFER_SIZE);
    }
    write(socket_fd, response_buffer, BUFFER_SIZE);
    return 0;
}

