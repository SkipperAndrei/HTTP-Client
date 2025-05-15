#include "utils.h"

int check_valid_integer(std::string number) {

    int result;

    try {
        result = std::stoi(number);
        return result;
    } catch (const std::invalid_argument& error) {
        fprintf(stderr, "ERROR: Invalid input\n");
        return -1;
    }

}

double check_valid_double(std::string number) {

    double rating;
    try {
        rating = std::stod(number);

        if (rating < 0 || rating > 9.9)
            throw std::out_of_range("ERROR: Rating must be between 0 and 9.9\n");
        
        return rating;
    } catch (const std::invalid_argument& error) {
        fprintf(stderr, "ERROR: Invalid rating\n");
        return -1;
    } catch (const std::out_of_range& error) {
        fprintf(stderr, "%s", error.what());
        return -1;
    }
}

void restart_connection() {
    close(sockfd);
    sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Connection to server failed...\n");
}

void append_movie_at_collection(char *cookie, char *jwt, std::string coll_id_s, int movie_id) {

    std::string url;
    url.append(COLLECTION_OPS_URL);
    url.append(coll_id_s);
    url.append(MOVIE_TERMINATOR);

    json payload;
    payload["id"] = movie_id;

    std::string serialized_json = payload.dump();
    char *ptr_payload = serialized_json.data();

    char *message;
    char *response;

    message = compute_post_request(SERVER_IP, url.data(), CONTENT_TYPE, &ptr_payload, 1, cookie, jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    // std::cout << response << "\n";

    if (strstr(response, "CREATED")) {
        std::cout << "SUCCESS: Movie added to collection successfully\n";
    } else {
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
    }

    free(message);
    free(response);
}

void extract_from_movie_json(char *response) {

    json response_json = json::parse(basic_extract_json_response(response));
    std::string string_rating(response_json["rating"]);
    std::string output = json{{"title", response_json["title"]}, 
                                {"year", response_json["year"]},
                                {"description", response_json["description"]},
                                {"rating", string_rating}}.dump(4);
    
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
    // std::cout << response << "\n";

    if (!strstr(response, "OK")) {
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
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
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
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
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
        free(message);
        free(response);
        return;
    }

    std::string string_payload(basic_extract_json_response(response));
    // std::cout << payload << "\n";

    json json_payload = json::parse(string_payload);

    for (auto &entry : json_payload["users"])
        std::cout << "#" << entry["id"] << " " << entry["username"].get<std::string>() << ":" << entry["password"].get<std::string>() << "\n";

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
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
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
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
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
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
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
    // std::cout << response << "\n";

    if (!strstr(response, "OK")) {
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
        free(message);
        free(response);
        return;
    }

    std::string string_payload(basic_extract_json_response(response));
    json json_payload = json::parse(string_payload);
    std::string jwt_token = json_payload["token"].get<std::string>();

    if (jwt) {
        strcpy(jwt, jwt_token.data());
        // std::cout << jwt << "\n";
    } else {
        jwt = new char[JWT_MAX_LENGTH];
        strcpy(jwt, jwt_token.data());
        // std::cout << jwt << "\n";
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

    if (!strstr(response, "OK")) {
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
        free(message);
        free(response);
        return;
    }

    /* Debug */
    // std::cout << response << "\n";

    std::cout << "SUCCESS: This is movie list\n";
    std::string string_payload(basic_extract_json_response(response));

    json json_payload = json::parse(string_payload);
    int cnt = 1;

    for (auto &entry : json_payload["movies"])
        std::cout << "#" << entry["id"] << " " << entry["title"].get<std::string>() << "\n"; 

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

    int integer_id = check_valid_integer(id_s);
    if (integer_id == -1)
        return;


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

    int year_id;
    double rating;

    year_id = check_valid_integer(year);
    if (year_id == -1)
        return;

    rating = check_valid_double(rating_s);
    if (rating == -1)
        return;

    if (title[0] == '\0' || description[0] == '\0') {
        fprintf(stderr, "ERROR: Invalid/Incomplete title or description\n");
        return;
    }

    json payload;
    payload["title"] = title;
    payload["year"] = year_id;
    payload["description"] = description;
    payload["rating"] = rating; 

    std::string serialized_json = payload.dump();
    char *ptr_payload = serialized_json.data();
    char *message;
    char *response;

    message = compute_post_request(SERVER_IP, ALL_MOVIES_URL, CONTENT_TYPE, &ptr_payload, 1, cookie, jwt);
    /* Debug */
    // std::cout << message << "\n";
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    // std::cout << response << "\n";

    if (strstr(response, "CREATED"))
        std::cout << "SUCCESS: Movie was added to the library\n";
    else
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));

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

    int integer_id = check_valid_integer(id_s);

    if (integer_id == -1)
        return;

    json payload;
    payload["id"] = integer_id;
    std::string serialized_json = payload.dump();
    char *ptr_payload = serialized_json.data();

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
    // std::cout << response << "\n";

    if (strstr(response, "OK"))
        std::cout << "SUCCESS: Movie was successfully deleted\n";
    else
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));

    free(message);
    free(response);
}

void update_movie(char *&cookie, char *&jwt) {
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

    int integer_id = check_valid_integer(id_s);
    int year_id = check_valid_integer(year);
    double rating = check_valid_double(rating_s);

    if (integer_id == -1 || year_id == -1 || rating == -1)
        return;

    if (title[0] == '\0' || description[0] == '\0') {
        fprintf(stderr, "ERROR: Invalid/Incomplete title/description\n");
        return;
    }
    
    std::string url;
    url.append(SPECIFIC_MOVIE_URL);
    url.append(id_s);

    json payload;
    payload["title"] = title;
    payload["year"] = year_id;
    payload["description"] = description;
    payload["rating"] = rating;


    std::string serialized_json = payload.dump();
    char *ptr_payload = serialized_json.data();

    char *message;
    char *response;

    message = compute_put_request(SERVER_IP, url.data(), CONTENT_TYPE, &ptr_payload, 1, cookie, jwt);
    send_to_server(sockfd, message);

    /* Debug */
    // std::cout << message << "\n";
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    // std::cout << response << "\n";

    if (strstr(response, "OK"))
        std::cout << "SUCCESS: Movie updated successfully\n";
    else
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
    
    free(response);
    free(message);
}

void get_collections(char *&cookie, char *&jwt) {
    return;
}

void get_collection(char *&cookie, char *&jwt) {
    return;
}

void add_collection(char *&cookie, char *&jwt) {
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

    std::cout << "num_movies=";
    std::string movies_s;
    std::getline(std::cin, movies_s);

    int nr_of_movies = check_valid_integer(movies_s);

    if (nr_of_movies == -1)
        return;

    std::vector<std::string> movie_ids(nr_of_movies + 1);
    bool invalid_input = false;

    for (int i = 0; i < nr_of_movies; i++) {
        std::cout << "movie_id[" << i << "]=";
        std::getline(std::cin, movie_ids[i]);
        if (check_valid_integer(movie_ids[i]) < 0)
            invalid_input = true;
    }

    if (invalid_input)
        return;

    char *message;
    char *response;

    json payload;
    payload["title"] = title;

    std::string serialized_payload = payload.dump();
    char *ptr_payload = serialized_payload.data();
    message = compute_post_request(SERVER_IP, BASE_COLLECTION_URL, CONTENT_TYPE, &ptr_payload, 1, cookie, jwt);

    // std::cout << message << "\n";
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    restart_connection();

    /* Debug */
    // std::cout << response << "\n";

    if (!strstr(response, "CREATED")) {
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
        free(message);
        free(response);
        return;
    }

    json returned_json = json::parse(basic_extract_json_response(response));
    int coll_id = returned_json["id"];

    // TODO: after making add_movie_to_collection
    for (int i = 0; i < nr_of_movies; i++) {
        append_movie_at_collection(cookie, jwt, std::to_string(coll_id), check_valid_integer(movie_ids[i]));
    }

    free(message);
    free(response);
}

void delete_collection(char *&cookie, char *&jwt) {
    return;
}

void add_movie_to_collection(char *&cookie, char *&jwt) {
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

    std::cout << "collection_id=";
    std::string coll_id_s;
    std::getline(std::cin, coll_id_s);

    std::cout << "movie_id=";
    std::string movie_id_s;
    std::getline(std::cin, movie_id_s);

    int movie_id = check_valid_integer(movie_id_s), coll_id = check_valid_integer(coll_id_s);

    if (coll_id < 0 || movie_id < 0) {
        return;
    }

    append_movie_at_collection(cookie, jwt, coll_id_s, movie_id);
 
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

    if (strstr(response, "OK")) {
        std::cout << "SUCCESS: User successfully logged out\n";
        delete[] cookie;
        cookie = nullptr;

        if (jwt) {
            delete[] jwt;
            jwt = nullptr;
        }

    } else {
        fprintf(stderr, "ERROR: %s\n", basic_extract_json_response(response));
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
