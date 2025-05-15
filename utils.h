#ifndef __UTILS__H__
#define __UTILS__H__

#include <iostream>
#include <cstring>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <stdexcept>
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

#define MAX_MOVIE_ID_DIGITS 10
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

/**
 * @brief This function checks if a string can be turned
 * into an integer.
 * @param number The string that can be turned into an int
 * @return The string parsed as int or -1 if it's an invalid string
 */
int check_valid_integer(std::string number);

/**
 * @brief This function checks if a string can be turned
 * into a double.
 * @param number The string that can be turned into a double
 * @return The string parsed as double or -1 if it's an invalid string
 */
double check_valid_double(std::string number);

/**
 * @brief This function extracts the movie information payload
 * of a HTTP response for output generation.
 * @param response The HTTP response from the server
 */
void extract_from_movie_json(char *response);

/**
 * @brief This function adds a new movie to
 * a collection given as parameter.
 * @param cookie The user cookie received at login
 * @param jwt The jwt received for library access
 * @param coll The id of the collection as string for URL
 * @param movie_id The id of the movie that needs to be added
 * @return true, if the movie could be added
 * @return false, if the movie couldn't be added
 */
bool append_movie_at_collection(char *cookie, char *jwt, std::string coll, int movie_id);

/**
 * @brief This function deletes a movie from
 * a collection given as parameter.
 * @param cookie The user cookie received at login
 * @param jwt The jwt received for library access
 * @param coll The id of the collection as string for URL
 * @param movie_id The id of the movie that needs to be added
 * @return true, if the movie was deleted
 * @return false, if the movie wasn't deleted
 */
void delete_from_collection(char *cookie, char *jwt, std::string coll_id_s, int movie_id);

/**
 * @brief This function is responsible for
 * restarting the connection to the server after
 * sending a request.
 */
void restart_connection();

/**
 * @brief This function implements the logic of
 * the log_admin command.
 * @attention Only admins can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void log_admin(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the add_user command.
 * @attention Only admins can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void add_user(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the get_users command.
 * @attention Only admins can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void get_users(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the delete_user command.
 * @attention Only admins can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void delete_users(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the logout_admin command.
 * @attention Only admins can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void logout_admin(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the login command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void login(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the get_access command.
 * This function will place in the jwt parameter the jwt for library access.
 * @attention Only users will receive a valid token upon calling this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void get_access(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the get_movies command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void get_movies(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the get_movie command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void get_movie(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the add_movie command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void add_movie(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the delete_movie command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void delete_movie(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the update_movie command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void update_movie(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the get_collections command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void get_collections(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the get_collection command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void get_collection(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the add_collection command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void add_collection(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the delete_collection command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void delete_collection(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the add_movie_to_collection command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void add_movie_to_collection(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the delete_movie_from_collection command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void delete_movie_from_collection(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the logout command.
 * @attention Only users can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void log_out(char *&cookie, char *&jwt);

/**
 * @brief This function implements the logic of
 * the exit command.
 * @attention Anyone can call this command.
 * @param cookie The user cookie received at login (nullable)
 * @param jwt The jwt token received for library access (nullable)
 */
void exit(char *&cookie, char *&jwt);

/**
 * @brief This function is used for initialising the client functionalities.
 * @attention The functionalities are stored in a HashMap with the command as the key and
 * a pointer to the command's corresponding function as value.
 * @param commands The command HashMap
 */
void build_functions(std::unordered_map<std::string, void(*)(char *&, char *&)> &commands);

#endif