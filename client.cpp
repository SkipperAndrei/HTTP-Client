#include "utils.h"

bool stop = false;
int sockfd;
bool logged_as_admin = false;

int main(int argc, char **argv) {

    sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Connection to server failed...\n");

    std::unordered_map<std::string, void(*)(char *&, char *&)> commands;
    build_functions(commands);

    std::string input;
    char *cookie = nullptr;
    char *jwt = nullptr;

    while (!stop) {
        std::getline(std::cin, input);

        if (commands.find(input) == commands.end()) {
            fprintf(stderr, "Invalid command, seek --help\n");
            continue;
        }

        auto function = commands[input];
        function(cookie, jwt);
    }

    if (cookie)
        delete[] cookie;

    if (jwt)
        delete[] jwt;

    close(sockfd);

    return 0;
}