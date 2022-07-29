#include <unistd.h>
#include <cstring>
#include <fstream>
#include "../include/server.h"
#include "../include/string_helper.h"

std::string bbfile;
std::string peers;
std::string config_file = "bbserv.conf";
int thmax = 20;
int bp = 9000;
int sp = 10000;
bool is_daemon = true;
bool debug_mode = false;

void sigterm_handler(int signum) {
//TODO
}

void sigquit_handler(int signum) {
//TODO
}

void sighup_handler(int signum) {
//TODO
}


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
                if (temp.at(1) == "true") {
                    is_daemon = true;
                } else if (temp.at(1) == "false") {
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
        exit(EXIT_FAILURE);
    }

    config_in.close();
}

int main(int argc, char *argv[]) {
    int opt;
//    signal(SIGTERM, sigterm_handler);
//    signal(SIGQUIT, sigquit_handler);
//    signal(SIGHUP, sighup_handler);

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

    std::map<std::string, std::string> args_map;

    args_map[THMAX] = std::to_string(thmax);
    args_map[BBPORT] = std::to_string(bp);
    args_map[SYNCPORT] = std::to_string(sp);
    args_map[BBFILE] = bbfile;
    args_map[PEERS] = peers;
    args_map[DAEMON] = std::to_string(is_daemon);
    args_map[DEBUG] = std::to_string(debug_mode);
    args_map[CONFIG_FILE] = config_file;

    printf("bbfile: %s, peers: %s, configfile: %s, thmax: %i, bp: %i, sp: %i, is_daemon: %i, debug: %i\n",
           bbfile.data(), peers.data(), config_file.data(), thmax, bp, sp, is_daemon, debug_mode);

    run_server(args_map);

//    auto print = (*printf);
    return 0;
}
