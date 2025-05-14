#include "utils.h"

void restart_connection() {
    close(sockfd);
    sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Connection to server failed...\n");
}

void extract_from_movie_json(char *response) {

    json response_json = json::parse(basic_extract_json_response(response));
    std::string string_rating(response_json["rating"]);
    double non_string_rating = atof(string_rating.data());
    std::string output = json{{"title", response_json["title"]}, 
                                {"year", response_json["year"]},
                                {"description", response_json["description"]},
                                {"rating", non_string_rating}}.dump();
    
    std::cout << "SUCCESS: We got the movie\n"; 
    std::cout << output.data() << "\n";
}

void log_admin(char*& cookie, char*& jwt) {

    if (cookie) {
        std::cout << "ERROR: Admin already logged in\n";
        return;
    }

    std::cout << "username=";
    std::string username;
    std::getline(std::cin, username);
    std::cout << "password=";
    std::string password;
    std::getline(std::cin, password);

    json payload;
    payload["username"] = username;
    payload["password"] = password;

    std::string serialized_json = payload.dump();
    char *ptr_payload = serialized_json.data();

    // /* Debug */
    // std::cout << ptr_payload << "\n";
    char *message;
    char *response;

    message = compute_post_request(SERVER_IP, ADMIN_LOGIN_URL, CONTENT_TYPE, &ptr_payload, 1, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    restart_connection();

    /* Debug */
    std::cout << response << "\n";

    if (!strstr(response, "OK")) {
        fprintf(stderr, "ERROR: Couldn't authenticate admin\n");
        free(message);
        free(response);
        return;
    } else {
        std::cout << "SUCCESS: Admin authentication was successfull\n";
    }

    char *cookie_line = strstr(response, "Set-Cookie:");

    if (!cookie_line) {
        fprintf(stderr, "ERROR: HTTP response doesn't have cookie line\n");
        free(message);
        free(response);
        return;
    }

    char pattern[] = "session=";
    char *cookie_beginning = strstr(cookie_line, pattern);

    if (cookie_beginning) {
        cookie_beginning = cookie_beginning + strlen(pattern);
        char *cookie_end = strchr(cookie_line, ';');

        if (!cookie_end) {
            fprintf(stderr, "ERROR: HTTP response has bad cookie delimiter\n");
            free(message);
            free(response);
            return;
        }

        std::string string_cookie(cookie_beginning, cookie_end);
        cookie = new char[COOKIE_LENGTH];
        strcpy(cookie, string_cookie.data());
        logged_as_admin = true;

        free(message);
        free(response);
    } else {
        fprintf(stderr, "ERROR: HTTP response doesn't have cookie value\n");
        free(message);
        free(response);
        return;
    }
}

void add_user(char *&cookie, char *&jwt) {
    
    if (!cookie) {
        fprintf(stderr, "ERROR: Command is requested by unknown user.\n");
        return;
    }

    /* Debug */
    // std::cout << cookie << "\n";

    if (!logged_as_admin) {
        fprintf(stderr, "ERROR: Command is requested by non-admin user\n");
        return;
    }

    std::cout << "username=";
    std::string username;
    std::getline(std::cin, username);
    std::cout << "password=";
    std::string password;
    std::getline(std::cin, password);

    if (username[0] == '\0' || password[0] == '\0') {
        fprintf(stderr, "ERROR: Incorrect/Incomplete information\n");
        return;
    }

    json payload;
    payload["username"] = username;
    payload["password"] = password;

    std::string serialized_json = payload.dump();
    char *ptr_payload = serialized_json.data();

    char *message;
    char *response = nullptr;

    message = compute_post_request(SERVER_IP, ADMIN_USER_ACTIONS_URL, CONTENT_TYPE, &ptr_payload, 1, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    restart_connection();

    /* Debug */
    // std::cout << response << "\n";

    if (!strstr(response, "CREATED"))
        fprintf(stderr, "ERROR: User couldn't be created\n");
    else
        std::cout << "SUCCESS: New user created\n";

    free(message);
    free(response);
}

void get_users(char *&cookie, char *&jwt) {

    if (!cookie) {
        fprintf(stderr, "ERROR: Command is requested by unknown user.\n");
        return;
    }

    if (!logged_as_admin) {
        fprintf(stderr, "ERROR: Command is requested by non-admin user.\n");
        return;
    }

    char *message;
    char *response;

    message = compute_get_request(SERVER_IP, ADMIN_USER_ACTIONS_URL, NULL, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    // std::cout << response << "\n";

    if (strstr(response, "OK")) {
        std::cout << "SUCCESS: Command returned list of users\n";
    } else {
        fprintf(stderr, "ERROR: Get users failed, try again...\n");
        free(message);
        free(response);
        return;
    }

    std::string string_payload(basic_extract_json_response(response));
    // std::cout << payload << "\n";

    json json_payload = json::parse(string_payload);

    for (auto &entry : json_payload["users"])
        std::cout << "#" << entry["id"] << " " << entry["username"] << ":" << entry["password"] << "\n";

    free(message);
    free(response);
}

void delete_users(char *&cookie, char *&jwt) {
    
    if (!cookie) {
        fprintf(stderr, "ERROR: You aren't logged in\n");
        return;
    }

    if (!logged_as_admin) {
        fprintf(stderr, "ERROR: Wrong command, try logout...\n");
        return;
    }

    std::cout << "username=";
    std::string username;
    std::getline(std::cin, username);

    if (username[0] == '\0') {
        fprintf(stderr, "ERROR: Invalid username, try again...\n");
        return;
    }

    char *message;
    char *response;

    std::string url;
    url.append(ADMIN_DELETE_USER_URL);
    url.append(username);
    
    /* Debug */
    // std::cout << url << "\n";

    message = compute_delete_request(SERVER_IP, (const char *) url.data(), NULL, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();
    
    /* Debug */
    // std::cout << response << "\n";

    if (strstr(response, "OK")) {
        std::cout << "SUCCESS: User " << username << " was deleted\n";
    } else {
        fprintf(stderr, "ERROR: The user couldn't be deleted\n");
    }

    free(message);
    free(response);
}

void logout_admin(char *&cookie, char *&jwt) {
    
    if (!cookie) {
        fprintf(stderr, "ERROR: You aren't logged in\n");
        return;
    }

    if (!logged_as_admin) {
        fprintf(stderr, "ERROR: Wrong command, try logout...\n");
        return;
    }

    char *message;
    char *response;
    message = compute_get_request(SERVER_IP, ADMIN_LOGOUT_URL, NULL, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* Debug */
    // std::cout << response << "\n";

    restart_connection();

    /* TODO: Add JWT logic */
    if (strstr(response, "OK")) {
        std::cout << "SUCCESS: Admin successfully logged out\n";
        delete[] cookie;
        cookie = nullptr;

        if (jwt) {
            delete[] jwt;
            jwt = nullptr;
        }

        logged_as_admin = false;
    } else {
        fprintf(stderr, "ERROR: Admin logout failed, try again\n");
    }

    free(message);
    free(response);
}

void login(char *&cookie, char *&jwt) {
    
    if (cookie) {
        fprintf(stderr, "ERROR: A user is already connected\n");
        return;
    }

    std::cout << "admin_username=";
    std::string admin;
    std::getline(std::cin, admin);
    std::cout << "username=";
    std::string username;
    std::getline(std::cin, username);
    std::cout << "password=";
    std::string password;
    std::getline(std::cin, password);

    json payload;
    payload["admin_username"] = admin;
    payload["username"] = username;
    payload["password"] = password;

    std::string serialized_json = payload.dump();
    char *ptr_payload = serialized_json.data();

    char *message;
    char *response;

    message = compute_post_request(SERVER_IP, USER_LOGIN_URL, CONTENT_TYPE, &ptr_payload, 1, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    restart_connection();

    /* Debug */
    // std::cout << response << "\n";

    if (!strstr(response, "OK")) {
        fprintf(stderr, "ERROR: Couldn't authenticate user\n");
        free(message);
        free(response);
        return;
    } else {
        std::cout << "SUCCESS: User authentication was successfull\n";
    }

    char *cookie_line = strstr(response, "Set-Cookie:");

    if (!cookie_line) {
        fprintf(stderr, "ERROR: HTTP response doesn't have cookie line\n");
        free(message);
        free(response);
        return;
    }

    char pattern[] = "session=";
    char *cookie_beginning = strstr(cookie_line, pattern);

    if (cookie_beginning) {
        cookie_beginning = cookie_beginning + strlen(pattern);
        char *cookie_end = strchr(cookie_line, ';');

        if (!cookie_end) {
            fprintf(stderr, "ERROR: HTTP response has bad cookie delimiter\n");
            free(message);
            free(response);
            return;
        }

        std::string string_cookie(cookie_beginning, cookie_end);
        cookie = new char[COOKIE_LENGTH];
        strcpy(cookie, string_cookie.data());
    } else {
        fprintf(stderr, "ERROR: HTTP response doesn't have cookie value\n");
    }

    free(message);
    free(response);
}

void get_access(char *&cookie, char *&jwt) {
    
    if (!cookie) {
        fprintf(stderr, "ERROR: Command requested by unknown user\n");
        return;
    }

    if (logged_as_admin) {
        fprintf(stderr, "ERROR: Command invalid for admin\n");
        return;
    }

    /* Debug */
    // std::cout << "Before : " << cookie << "\n";

    char *message;
    char *response;

    message = compute_get_request(SERVER_IP, LIBRARY_ACCESS_URL, NULL, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    std::cout << response << "\n";

    if (!strstr(response, "OK")) {
        fprintf(stderr, "ERROR: Get access command failed...\n");
        free(message);
        free(response);
        return;
    }

    std::string string_payload(basic_extract_json_response(response));
    json json_payload = json::parse(string_payload);

    if (jwt) {
        strcpy(jwt, json_payload["token"].dump().data() + 1);
        jwt[strlen(jwt) - 1] = '\0';
        std::cout << jwt << "\n";
    } else {
        jwt = new char[JWT_MAX_LENGTH];
        strcpy(jwt, json_payload["token"].dump().data() + 1);
        jwt[strlen(jwt) - 1] = '\0';
        std::cout << jwt << "\n";
    }

    /* Debug */
    // std::cout << "After : " << cookie << "\n";

    std::cout << "SUCCESS: JWT received from server\n";

    free(message);
    free(response);
}

void get_movies(char *&cookie, char *&jwt) {
    if (!cookie) {
        fprintf(stderr, "ERROR: Command requested by unknown user\n");
        return;
    }

    if (logged_as_admin) {
        fprintf(stderr, "ERROR: Command invalid for admin\n");
        return;
    }

    if (!jwt) {
        fprintf(stderr, "ERROR: Access token invalid or expired, try again...\n");
        return;
    }

    char *message;
    char *response;

    message = compute_get_request(SERVER_IP, ALL_MOVIES_URL, NULL, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    std::cout << response << "\n";

    free(message);
    free(response);

}

void get_movie(char *&cookie, char *&jwt) {
    
    if (!cookie) {
        fprintf(stderr, "ERROR: Command requested by unknown user\n");
        return;
    }

    if (logged_as_admin) {
        fprintf(stderr, "ERROR: Command invalid for admin\n");
        return;
    }

    if (!jwt) {
        fprintf(stderr, "ERROR: Access token invalid or expired, try again...\n");
        return;
    }

    std::cout << "id=";
    std::string id_s;
    std::getline(std::cin, id_s);

    char *message;
    char *response;
    std::string movie_url;
    movie_url.append(SPECIFIC_MOVIE_URL);
    movie_url.append(id_s.data());

    /* Debug */
    // std::cout << movie_url << "\n";

    message = compute_get_request(SERVER_IP, movie_url.data(), NULL, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    // std::cout << response << "\n";

    if (strstr(response, "OK"))
        extract_from_movie_json(response);
    else
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));

    free(message);
    free(response);

}

void add_movie(char *&cookie, char *&jwt) {
    
    if (!cookie) {
        fprintf(stderr, "ERROR: Command requested by unknown user\n");
        return;
    }

    if (logged_as_admin) {
        fprintf(stderr, "ERROR: Command invalid for admin\n");
        return;
    }

    if (!jwt) {
        fprintf(stderr, "ERROR: Access token invalid or expired, try again...\n");
        return;
    }

    std::cout << "title=";
    std::string title;
    std::getline(std::cin, title);
    std::cout << "year=";
    std::string year;
    std::getline(std::cin, year);
    std::cout << "description=";
    std::string description;
    std::getline(std::cin, description);
    std::cout << "rating=";
    std::string rating_s;
    std::getline(std::cin, rating_s);
    double rating = atof(rating_s.data());

    if (title[0] == '\0' || year[0] == '\0' || description[0] == '\0' || rating < 0 || rating > 9.9) {
        fprintf(stderr, "ERROR: Invalid movie input, try again\n");
        return;
    }

    json payload;
    payload["title"] = title;
    payload["year"] = atoi(year.data());
    payload["description"] = description;
    payload["rating"] = rating; 

    std::string serialized_json = payload.dump();
    char *ptr_payload = serialized_json.data();    

    char *message;
    char *response;

    message = compute_post_request(SERVER_IP, ALL_MOVIES_URL, CONTENT_TYPE, &ptr_payload, 1, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    // std::cout << response << "\n";

    if (strstr(response, "CREATED"))
        std::cout << "SUCCESS: Movie was added to the library\n";
    else
        fprintf(stderr, "ERROR: Movie couldn't be added to the library\n");

    free(message);
    free(response);

}

void delete_movie(char *&cookie, char *&jwt) {
    if (!cookie) {
        fprintf(stderr, "ERROR: Command requested by unknown user\n");
        return;
    }

    if (logged_as_admin) {
        fprintf(stderr, "ERROR: Command invalid for admin\n");
        return;
    }

    if (!jwt) {
        fprintf(stderr, "ERROR: Access token invalid or expired, try again...\n");
        return;
    }

    std::cout << "id=";
    std::string id_s;
    std::getline(std::cin, id_s);

    if (id_s[0] == '\0') {
        fprintf(stderr, "ERROR: Invalid movie id, try again\n");
        return;
    }

    json payload;
    payload["id"] = atoi(id_s.data());
    char *ptr_payload = payload.dump().data();

    char *message;
    char *response;

    std::string url;
    url.append(SPECIFIC_MOVIE_URL);
    url.append(id_s);

    message = compute_delete_request(SERVER_IP, url.data(), NULL, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    /* TODO: finish it later after get_movies */
    std::cout << response << "\n";

    free(message);
    free(response);

}

void update_movie(char *&cookie, char *&jwt) {
    return;
}

void get_collections(char *&cookie, char *&jwt) {
    return;
}

void get_collection(char *&cookie, char *&jwt) {
    return;
}

void add_collection(char *&cookie, char *&jwt) {
    return;
}

void delete_collection(char *&cookie, char *&jwt) {
    return;
}

void add_movie_to_collection(char *&cookie, char *&jwt) {
    return;
}

void delete_movie_to_collection(char *&cookie, char *&jwt) {
    return;
}

void log_out(char *&cookie, char *&jwt) {
    
    if (!cookie) {
        fprintf(stderr, "ERROR: No users logged in...\n");
        return;
    }

    if (logged_as_admin) {
        fprintf(stderr, "ERROR: Wrong command, try logout_admin...\n");
        return;
    }

    char *message;
    char *response;
    message = compute_get_request(SERVER_IP, USER_LOGOUT_URL, NULL, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* Debug */
    // std::cout << response << "\n";

    restart_connection();

    /* TODO: Add JWT logic */
    if (strstr(response, "OK")) {
        std::cout << "SUCCESS: User successfully logged out\n";
        delete[] cookie;
        cookie = nullptr;

        if (jwt) {
            delete[] jwt;
            jwt = nullptr;
        }

    } else {
        fprintf(stderr, "ERROR: User logout failed, try again\n");
    }

    free(message);
    free(response);

}

void exit(char *&cookie, char *&jwt) {
    stop = true;
    return;
}

void build_functions(std::unordered_map<std::string, void(*)(char *&, char *&)> &commands) {

    commands["login_admin"] = &log_admin;
    commands["add_user"] = &add_user;
    commands["get_users"] = &get_users;
    commands["delete_user"] = &delete_users;
    commands["logout_admin"] = &logout_admin;
    commands["login"] = &login;
    commands["get_access"] = &get_access;
    commands["get_movies"] = &get_movies;
    commands["get_movie"] = &get_movie;
    commands["add_movie"] = &add_movie;
    commands["delete_movie"] = &delete_movie;
    commands["update_movie"] = &update_movie;
    commands["get_collections"] = &get_collections;
    commands["get_collection"] = &get_collection;
    commands["add_collection"] = &add_collection;
    commands["delete_collection"] = &delete_collection;
    commands["add_movie_to_collection"] = &add_movie_to_collection;
    commands["delete_movie_to_collection"] = &delete_movie_to_collection;
    commands["logout"] = &log_out;
    commands["exit"] = &exit;   
}
