//
// Created by jj on 22/07/22.
//

#ifndef BBSERVER2_DB_FILE_UTIL_H
#define BBSERVER2_DB_FILE_UTIL_H

#include <atomic>
#include <fstream>
#include <semaphore>

#define READ_WAIT 3
#define WRITE_WAIT 6

extern std::atomic<int> readers_count;
extern std::atomic<int> writers_count;
extern std::binary_semaphore read_semaphore;
extern std::binary_semaphore write_semaphore;

extern std::fstream get_stream(const std::string &db_file);

extern std::string read_message(const std::string &db_file, const std::string &msg);

extern std::string read_recent_line(const std::string &db_file);

extern int write_message(const std::string &db_file, std::string &poster, std::string msg);

extern int replace_message(const std::string &db_file, const std::string &poster, std::string msg);

extern void lock_acquire();

extern void lock_release();

#endif //BBSERVER2_DB_FILE_UTIL_H
