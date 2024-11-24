#include <stdio.h>

#include "sock.h"
#include "http_server.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("usage: %s port [max_connections]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *port = argv[1];
    if (isdigit(atoi(port)) != 0) {
        printf("invalid port provided");
        return EXIT_FAILURE;
    }

    printf("starting http web server http://0.0.0.0:%s ...\n", port);
    // start_http_server("127.0.0.1", 8080);


    // printf("starting server socket on 0.0.0.0:%s ...\n", port);
    
    SOCKET serverSocket = create_sock(port);
    if (serverSocket == INVALID_SOCKET) {
        sock_destroy(serverSocket);
        return EXIT_FAILURE;
    }

    if (sock_listen(serverSocket) != 0){
        // unable to listen on the specified port
        sock_destroy(serverSocket);
        return EXIT_FAILURE;
    }


    // actually start the server (multiple clients supported)
    int maxConnections = DEFAULT_MAX_CONNECTIONS;

    if (argc > 2){
        if (isdigit(atoi(argv[2])) != 0 ){
            printf("invalid number for max_connections: %s\n", argv[2]);
            sock_destroy(serverSocket);
            return EXIT_FAILURE;
        }

        maxConnections = atoi(argv[2]);
    }


    //--- HTTP SERVER ---//
    int error = 0;
    http_server server = create_http_server(&serverSocket, maxConnections, &error);

    if (error != 0){
        printf("unable to start a new http server\n");
        return EXIT_FAILURE;
    }

    printf("web server '%s' running on 0.0.0.0:%s ...\n", server.serverName, port);
    printf("press CTRL+C to close the server\n");
    start_http_server(&server);
    
    // this will be executed after we stop the server with a CTRL+C
    // this will also close `serverSocket` so we don't have to worry about it
    if (close_http_server(&server) != 0){
        printf("error while closing the server\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

