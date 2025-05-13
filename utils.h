#ifndef __UTILS__H__
#define __UTILS__H__

#include <iostream>
#include <cstring>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <errno.h>
#include <unordered_map>
#include "helpers.h"
#include "json.hpp"
#include "urls.h"
#include "requests.h"

using json = nlohmann::json;

#define MAX_URL_LENGTH 150
#define COOKIE_LENGTH 500
#define JWT_MAX_LENGTH 700
#define MAX_STDIN_INPUT 1000
#define SERVER_PORT 8081
#define SERVER_IP "63.32.125.183"
#define CONTENT_TYPE "application/json"

extern bool stop;
extern int sockfd;
extern bool logged_as_admin;

#define DIE(condition, message, ...) \
	do { \
		if ((condition)) { \
			fprintf(stderr, "[(%s:%d)]: " # message "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
			perror(""); \
			exit(1); \
		} \
	} while (0)

void restart_connection();
void log_admin(char *&cookie, char *&jwt);
void add_user(char *&cookie, char *&jwt);
void get_users(char *&cookie, char *&jwt);
void delete_users(char *&cookie, char *&jwt);
void logout_admin(char *&cookie, char *&jwt);
void login(char *&cookie, char *&jwt);
void get_access(char *&cookie, char *&jwt);
void get_movies(char *&cookie, char *&jwt);
void get_movie(char *&cookie, char *&jwt);
void add_movie(char *&cookie, char *&jwt);
void delete_movie(char *&cookie, char *&jwt);
void update_movie(char *&cookie, char *&jwt);
void get_collections(char *&cookie, char *&jwt);
void get_collection(char *&cookie, char *&jwt);
void add_collection(char *&cookie, char *&jwt);
void delete_collection(char *&cookie, char *&jwt);
void add_movie_to_collection(char *&cookie, char *&jwt);
void delete_movie_to_collection(char *&cookie, char *&jwt);
void log_out(char *&cookie, char *&jwt);
void exit(char *&cookie, char *&jwt);
void build_functions(std::unordered_map<std::string, void(*)(char *&, char *&)> &commands);

#endif