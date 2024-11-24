#pragma once

#include <string.h>
#include <unistd.h>

#include "sock.h"
#include "structs.h"

#ifndef DEFAULT_MAX_CONNECTIONS
    // default maximum number of concurrent connections
    #define DEFAULT_MAX_CONNECTIONS 1024
#endif

// create a new web server on the specified address
// @param server_socket a pointer a socket where the server will bind
// @param max_connections maximum number of connections that the server can handle
// @param error if the value returned is != 0, there has been an error while creating the server
// @returns a new `http_server` struct with all the server's informations
http_server create_http_server(SOCKET *server_socket, DWORD max_connections, int *error);

// start the web server
// @param server the web server to start 
void start_http_server(http_server* server);

// shut down the web server (and the internal socket)
// @param http_server the server to shut down
// @returns a non zero value to signal an error 
int close_http_server(http_server *http_server);


