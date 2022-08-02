//
// Created by jj on 19/07/22.
//

#include <cstring>
#include <chrono>
#include <thread>
#include "../include/db_file_util.h"
#include "../include/string_helper.h"

std::atomic<int> readers_count = 0;
std::atomic<int> writers_count = 0;
std::binary_semaphore read_semaphore{1};
std::binary_semaphore write_semaphore{1};


std::string read_message(const std::string &db_file, const std::string &msg) {
    read_semaphore.acquire();
    readers_count++;
    if (readers_count == 1) {
        write_semaphore.acquire();
    }
    read_semaphore.release();
    printf("Readers count: %i\n", (int) readers_count);
    std::ifstream file(db_file);
    auto msg_no = string_split(msg, ' ')[1];
    std::string line, temp_msg_no;
    while (file) {
        getline(file, line);
        if (line.empty() or string_startswith(line, " "))
            continue;
        temp_msg_no = string_file_msg_split(line, '/', 1)[0];
        if (!strcmp(msg_no.data(), temp_msg_no.data())) {
            break;
        }
    }
    file.close();
    std::this_thread::sleep_for(std::chrono::seconds(READ_WAIT));

    read_semaphore.acquire();
    readers_count--;
    if (readers_count == 0) {
        write_semaphore.release();
    }
    printf("Readers count: %i\n", (int) readers_count);
    read_semaphore.release();
    return line;
}

std::string read_recent_line(const std::string &db_file) {
    std::ifstream file(db_file);
    std::string line;
    std::string temp;
    int msg_no = -1;
    int curr_msg_no;
    while (file) {
        getline(file, temp);
        if (temp.empty() or string_startswith(temp, " "))
            continue;
        curr_msg_no = std::stoi(string_split(temp, '/')[0]);
        if (curr_msg_no > msg_no) {
            msg_no = curr_msg_no;
            line = temp;
        }
    }
    file.close();
    return line;
}

int write_message(const std::string &db_file, std::string &poster, std::string msg) {
    write_semaphore.acquire();
    writers_count++;
    printf("Writers count %i\n", (int) writers_count);
    msg = string_file_msg_split(msg, ' ', 1)[1];
    std::string line = read_recent_line(db_file);
    int msg_no = line.empty() ? 0 : std::stoi(string_split(line, '/')[0]);
    line = std::to_string(++msg_no) + "/" + poster + "/" + msg;
//    auto file = get_stream(db_file);
    std::ofstream file(db_file, std::ios::app);
    if (file) {
        file << line << std::endl;
    } else {
        printf("Filed to write the message!\n");
        msg_no = -1;
    }
    file.close();
    std::this_thread::sleep_for(std::chrono::seconds(WRITE_WAIT));
    writers_count--;
    printf("Writers count %i\n", (int) writers_count);
    write_semaphore.release();
    return msg_no;
}

int replace_message(const std::string &db_file, const std::string &poster, std::string msg) {
    write_semaphore.acquire();
    writers_count++;
    printf("Writers count %i\n", (int) writers_count);
    msg = string_file_msg_split(msg, ' ', 1)[1];
    auto v = string_file_msg_split(msg, '/', 1);
    auto msg_no = v[0];
    msg = v[1];
//    std::fstream file(db_file, std::ios::out | std::ios::in);
    std::fstream file(db_file);
    std::string line, temp_msg_no, replacement = msg_no + "/" + poster + "/" + msg;
    bool found = false;
    while (file) {
        getline(file, line);
        if (line.empty() or string_startswith(line, " "))
            continue;
        temp_msg_no = string_file_msg_split(line, '/', 1)[0];
        if (!strcmp(temp_msg_no.data(), msg_no.data())) {
            found = true;
            char d[1];
            d[0] = ' ';
            long c_pos = (long) file.tellp();
            long i_pos = c_pos - line.length() - 1;
            file.seekp(i_pos);
            for (int i = 0; i < line.length(); ++i) {
                file.write(d, 1);
            }
            file.seekp(0, std::ios::end);
            file << replacement << std::endl;
            break;
        }
    }
    file.close();
    std::this_thread::sleep_for(std::chrono::seconds(WRITE_WAIT));
    writers_count--;
    printf("Writers count %i\n", (int) writers_count);
    write_semaphore.release();
    return found ? std::stoi(msg_no) : -1;
}

void lock_acquire() {
    read_semaphore.acquire();
    write_semaphore.acquire();
}

void lock_release() {
    read_semaphore.release();
    write_semaphore.release();
}