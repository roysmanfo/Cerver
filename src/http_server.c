
/*
    create an http server that runs on the specified port.
    Support for GET requests 
*/


#include "http_server.h"
#include "requests.h"





//* === GLOBAL VARIABLES ===

// yea, this tells you that the server is runnin'
volatile BOOL isServerRunning;

// connection parameters (passed to HandleConnection)
struct cparams{
    http_server *server;
    SOCKET client;
};


//* === HELPERS (not exportd) ===

// thread function that handles a single connection
DWORD WINAPI HandleConnection(LPVOID lpParam) {
    struct cparams params = *(struct cparams*) lpParam;
    SOCKET client = (SOCKET) params.client;
    http_server *server = (http_server*) params.server;
    
    char buffer[DEFAULT_BUFLEN];

    BOOL closeRequestByClient = FALSE;

    while (!closeRequestByClient && isServerRunning) {
        int result = sock_recv(client, buffer, DEFAULT_BUFLEN);
        if (result <= 0) {
            closeRequestByClient = TRUE;
            break;
        }

        char *res = handle_request(buffer, server);

        // should handle the request, but for now just echo what the client said
        send(client, res, strlen(res), 0);

        // check if the client asked to close the connection
        Result lastConnRes = get_last_connection_results();
        if (lastConnRes.close){
            closeRequestByClient = TRUE;
        }
    }
    
    // every thread is responsible for closing it's own connection
    if (!isServerRunning){
        int iResult = shutdown(client, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("[Err] shutdown failed: %d\n", WSAGetLastError());
            closesocket(client);
            free(lpParam);
            return EXIT_FAILURE;
        }
    }

    closesocket(client);
    free(lpParam);

    return EXIT_SUCCESS;
}

// detects a signal and
BOOL WINAPI SignalHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        printf("\n[!] CTRL+C detected, shutting down the server...\n");
        isServerRunning = FALSE;
        return TRUE;
    }
    return FALSE;
}


//* === MAIN FUNCTIONS (exported) ===

http_server create_http_server(SOCKET *server_socket, DWORD max_connections, int *error){
    http_server *server = (http_server *) malloc(sizeof(http_server));
    if (server == NULL){
        printf("unable to start a new web server\n");
        *error = 1;
        return *server;
    }

    server->maxConnections = max_connections;

    HANDLE _hthreads[max_connections];
    server->hThreads = _hthreads;

    DWORD _hthreadids[max_connections];
    server->hThreadIds = _hthreadids;

    server->socket = *server_socket;
    server->serverName = "Cerver";
    server->version = "1.1";
    server->numConnections = 0;
    
    isServerRunning = TRUE;
    return *server;
}

void start_http_server(http_server *server){
    // set the control handler
    // this should avoid any resource leak
    // as it is used to safely shutdown the server
    if (!SetConsoleCtrlHandler(SignalHandler, TRUE)) {
        printf("[!] could not set control handler\n");
        return;
    }

    while(isServerRunning){
        SOCKET clientSock = sock_accept(server->socket);
        if (clientSock == INVALID_SOCKET){
            printf("[!] error while accepting a new client\n");
            sock_destroy(server->socket);
            continue;
        }

        // if the server is full, whait for someone to leave
        if (server->numConnections == server->maxConnections){
            printf("[!] the server has reached its max capacity (%ld clients connected)\n", server->numConnections);
            // wait for a single object
            DWORD pos = WaitForMultipleObjects(server->numConnections, server->hThreads, FALSE, INFINITE);
            pos -= WAIT_ABANDONED_0; // position of the closed connection 
            server->numConnections--;
        }

        // create a client pointer for the thread
        struct cparams *params = (struct cparams*) malloc(sizeof(struct cparams));
        // SOCKET* client = (SOCKET*) malloc(sizeof(SOCKET));

        if (params == NULL) {
            printf("Memory allocation failed\n");
            closesocket(clientSock);
            continue;
        }
        params->client = clientSock;
        params->server = server;

        // handle connection on a separate thread
        server->hThreads[server->numConnections] = CreateThread(
            NULL, 
            0, 
            HandleConnection,
            params, 
            0, 
            &server->hThreadIds[server->numConnections]
        );

        if (server->hThreads[server->numConnections] == NULL) {
            // unable to start a new thread
            printf("[-] Error creating thread n.%ld\n", server->numConnections);
            sock_destroy(clientSock);
            free(params);
        } else {
            server->numConnections++;
        }
    }
}


int close_http_server(http_server *http_server){
    if (http_server != NULL){
        isServerRunning = FALSE;
        int err = sock_destroy(http_server->socket);
        free(http_server);
        return err;
    }
    return 0;
}



