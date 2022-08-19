#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <csignal>
#include <map>
#include <sys/stat.h>
#include "../include/common.h"
#include "../include/connect_client.h"
#include "../include/connect_peer.h"
#include "../include/server.h"
#include "../include/string_helper.h"

#define PID_FILE "bbserv.pid"
#define LOG_FILE "bbserv.log"

std::string bbfile;
std::string peers;
std::string config_file = "bbserv.conf";
int thmax = 20;
int bp = 9000;
int sp = 10000;
bool is_daemon = true;
bool debug_mode = false;

std::map<std::string, std::string> args_map;
// auto lfile = freopen(LOG_FILE, "a", stdout);


void create_daemon();

//Method to read config file
void read_config() {
    std::ifstream config_in(config_file);

    if (config_in) {
        while (config_in) {
            std::string line;
            getline(config_in, line);
            if (line.empty())
                continue;

            auto temp = string_split(line, '=');

            if (temp.size() != 2 && !line.empty()) {
                printf("Invalid text in %s : %s\n", config_file.data(), line.data());
                exit(EXIT_FAILURE);
            }

            if (temp[0] == THMAX) {
                thmax = std::stoi(temp.at(1));
            } else if (temp[0] == BBPORT) {
                bp = std::stoi(temp.at(1));
            } else if (temp[0] == SYNCPORT) {
                sp = std::stoi(temp.at(1));
            } else if (temp[0] == BBFILE) {
                bbfile = temp.at(1);
            } else if (temp[0] == PEERS) {
                peers = temp.at(1);
            } else if (temp[0] == DAEMON) {
                if (temp.at(1) == "true" || temp.at(1) == "1") {
                    is_daemon = true;
                } else if (temp.at(1) == "false" || temp.at(1) == "0") {
                    is_daemon = false;
                } else {
                    printf("Invalid %s value in %s: %s", DAEMON, config_file.data(), temp.at(1).data());
                    exit(EXIT_FAILURE);
                }
            } else if (temp[0] == DEBUG) {
                if (temp.at(1) == "true") {
                    debug_mode = true;
                } else if (temp.at(1) == "false") {
                    debug_mode = false;
                } else {
                    printf("Invalid %s value in %s: %s", DEBUG, config_file.data(), temp.at(1).data());
                    exit(EXIT_FAILURE);
                }
            } else {
                printf("Invalid key in %s: %s\n", config_file.data(), temp.at(0).data());
                exit(EXIT_FAILURE);
            }
        }
    } else {
        printf("Failed to open %s\n Error code: %i", config_file.data(), errno);
//        exit(EXIT_FAILURE);
    }

    config_in.close();
}

//Method to write pid in bbserv.pid
void write_pid() {
    std::ofstream file(PID_FILE);
    //    printf("PID: %d\nPPID: %d\n", getpid(), getppid());
    if (file) {
//        file << "PID: " << getpid() << "\nPPID: " << getppid() << std::endl;
        file << getpid() << std::endl;
    } else {
        if (debug_mode) {
            printf("Unable to create/write %s\n", PID_FILE);
        }
    }
    file.close();
}

//SIGTERM Handler
void sigterm_handler(int signum) {
    printf("SIGTERM received. Terminating\n");
    exit(EXIT_SUCCESS);
}

//SIGQUIT Handler
void sigquit_handler(int signum) {
    printf("SIGQUIT received. Terminating\n");
    exit(EXIT_SUCCESS);
}

//SIQHUP Handler
void sighup_handler(int signum) {
    printf("SIGHUP received. Restarting...\n");
    read_config();

    args_map[THMAX] = std::to_string(thmax);
    args_map[BBFILE] = bbfile;
    args_map[PEERS] = peers;
    args_map[DEBUG] = std::to_string(debug_mode);
    args_map[CONFIG_FILE] = config_file;
    args_map[SYNCPORT] = std::to_string(sp);
    args_map[BBPORT] = std::to_string(bp);

    printf("bbfile: %s, peers: %s, configfile: %s, thmax: %i, bp: %i, sp: %i, is_daemon: %i, debug: %i\n",
           bbfile.data(), peers.data(), config_file.data(), thmax, bp, sp, is_daemon, debug_mode);

    master_socket_close();
    master_socket_peer_close();
    run_server(args_map);
}

//Method to set Unix signals
void set_signals() {
    signal(SIGTERM, sigterm_handler);
    signal(SIGQUIT, sigquit_handler);
    signal(SIGHUP, sighup_handler);
}

//Method to create daemon using two fork method
void create_daemon() {
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        printf("%s\n", "1st forking failed to create a daemon. Exiting..\n");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        printf("%s\n", "1st forking success! Terminating the parent process..\n");
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    set_signals();

    pid = fork();
    if (pid < 0) {
        printf("%s\n", "2nd forking failed to create a daemon. Exiting..\n");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        printf("%s\n", "2nd forking forking success! Terminating the parent process..\n");
        exit(EXIT_SUCCESS);
    }

    umask(0);
    for (auto x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }
    freopen(LOG_FILE, "w+", stdout);
}

int main(int argc, char *argv[]) {
    int opt;
    std::string uf_peers;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-c")) {
            config_file = argv[i + 1];
            break;
        }
    }

    read_config();

    while ((opt = getopt(argc, argv, ":b:fdT:tp:s:c:")) != -1) {
        switch (opt) {
            case 'b':
                bbfile = optarg;
                printf("Overriding the database file with %s\n", bbfile.data());
                break;
            case 'f':
                is_daemon = false;
                printf("DAEMON mode disabled\n");
                break;
            case 'd':
                debug_mode = true;
                printf("Enabled debugging facilities\n");
                break;
            case 'T':
                thmax = std::stoi(optarg);
                printf("Overriding the number of pre allocated threads with %i\n", thmax);
                break;
            case 't':
                // printf("is f %s\n", optarg);
                break;
            case 'p':
                bp = std::stoi(optarg);
                printf("Overriding the port number for handle_client-server communication with %i\n", bp);
                break;
            case 's':
                sp = std::stoi(optarg);
                printf("Overriding the port number for inter-server communication with %i\n", sp);
                break;
            case 'c':
                config_file = optarg;
                printf("Overriding the config file with %s\n", config_file.data());
                break;
            case ':':
                printf("option needs a value\n");
                break;
            case '?':
                printf("unknown option: %c\n", optopt);
                break;
            default:
                printf("is default\n");
                break;
        }
    }

    args_map[THMAX] = std::to_string(thmax);
    args_map[BBPORT] = std::to_string(bp);
    args_map[SYNCPORT] = std::to_string(sp);
    args_map[BBFILE] = bbfile;
    args_map[PEERS] = peers;
    args_map[DAEMON] = std::to_string(is_daemon);
    args_map[DEBUG] = std::to_string(debug_mode);
    args_map[CONFIG_FILE] = config_file;

    for (int i = optind; i < argc; ++i) {
        std::string arg = argv[i];
        bool flag = false;
        if (string_startswith(arg, "localhost:") or string_startswith(arg, "127.0.0.1:")) {
            uf_peers += "127.0.0.1:" + string_split(arg, ':')[1] + " ";
            flag = true;
        }
        if(flag){
            args_map[PEERS] = uf_peers;
        }
    }

    if (debug_mode) {
        printf("bbfile: %s, peers_set: %s, configfile: %s, thmax: %s, bp: %s, sp: %s, is_daemon: %s, debug: %s\n",
               args_map[BBFILE].data(), args_map[PEERS].data(), args_map[CONFIG_FILE].data(), args_map[THMAX].data(),
               args_map[BBPORT].data(), args_map[SYNCPORT].data(), args_map[DAEMON].data(), args_map[DEBUG].data());
    }

    if (bbfile.empty()) {
        printf("Failed to obtain the database file location\n");
        exit(EXIT_FAILURE);
    }

    if(is_daemon){
        create_daemon();
    } else{
        set_signals();
    }

    write_pid();
    run_server(args_map); //Calling the server
    return 0;
}
