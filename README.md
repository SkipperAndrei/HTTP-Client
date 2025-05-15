# HTTP Client
##### Made by: Giurgiu Andrei-Stefan 325CA

## Project idea
* This project represents the implementation of a HTTP client over a movie-library service.
* The main idea is that the normal users can store, receive or delete movie informations on the server and even group those movies into collections.
* In order to gain access to a user-account on the server, an admin user will have to create the account and will be responsible for the user's actions on the server.
* When logging in, doesn't matter whether it's an admin or a normal user, the user will receive a cookie from the server.

## Commands
* In order to write clean-code in the main function of the client, I decided to have a HashMap where i will keep pairs of (command, pointer to a function that executes the command).
* Since most commands require the cookie and/or JWT, the signature of the functions that execute commands is the following:
```
    void function (char *&, char *&) // function is a placeholder for the function name
```
* Parameters are sent via reference because, initially, both the cookie and the jwt are set on NULL, since no user was logged in when the client started.
* There are commands that will allocate memory for the cookie and jwt when there is necessary => <b><i>login, login_admin, get_access</b></i>.
* After the client connects to the server, the command map is initialised with the allowed commands.
* In order to execute the commmands, the client should:
    * Have a cookie linked to the current session
    * Have administrator status for user-managing commands
    * Have a JWT for commands that involve the library
* Apart from the login commands, all commands require a cookie to be put in the HTTP header, in order to be executed.

## Administrator user
* The administrator user has the role of managing users on the server.
* In order to validate the permissions for issuing admin-only commands, the program keeps a flag that will be set on true when an admin is logged on the client.
* The possible commands that an admin can issue on the server are:
``` 
    login_admin -> for logging on the admin account
    add_user -> for adding a new user on the server
    get_users -> to print the users that are linked to the admin account that issued the command
    delete_user -> for deleting an existing user from the server
    logout_admin -> to logout from the admin account
```

## Regular user
* A regular user can request access to the movie-library stored on the server.
* After processing the request and checking permissions, the user will be provided a JWT that will be placed in the HTTP header of every command involving the library.
* Without the JWT, the server will not allow the execution of any command on the library.
* The possible commands that a regular user can issue on the server/library are:
```
    login -> to login on the regular user account
    logout -> to logout from the regular user account
    get_movies -> reveals all movies added by the user
    get_movie -> reveals information about a movie added by the user
    update_movie -> updates information about the movie added by the user
    delete_movie -> deletes a movie from the library
    get_collections -> reveals all collections created by the user
    get_collection -> reveals information about a collection created by the user
    add_collection -> creates a new collection and links it to the user
    delete_collection -> deletes an entire collection
    add_movie_to_collection -> adds a specific movie to a collection
    delete_movie_from_collection -> deletes a specific movie from a collection
```
* The <b> exit </b> command is the only command that can be called by both the admin and the regular user and execute successfully.

## References
* I started the project from the code provided in the ninth lab of the Communication Protocols course https://pcom.pages.upb.ro/labs/lab9/exercises.html.
* From the lab, I took the examples of creating GET and POST requests and based on these I created custom functions for PUT and DELETE requests.

