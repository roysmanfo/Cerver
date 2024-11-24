
/*
    create a simple server socket on the specified port
    equivalent of `nc -lvnp {port}`, but also the ip address to b 
*/


#include "sock.h"

SOCKET create_sock(char* port) {
    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return INVALID_SOCKET;
    }

    struct addrinfo *result = NULL, hints;
    
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return INVALID_SOCKET;
    }

    SOCKET sock = INVALID_SOCKET;
    sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        printf("unable to create a new socket: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    iResult = bind(sock, result->ai_addr, (int) result->ai_addrlen);
    if (iResult != 0) {
        printf("unable to bind on port %s\n", port);
        freeaddrinfo(result);
        sock_destroy(sock);
        return INVALID_SOCKET;
    }

    freeaddrinfo(result);
    return sock;
}


int sock_destroy(SOCKET sock){
    closesocket(sock);
    WSACleanup();
    return 0;
}

int sock_listen(SOCKET sock){
    if (listen( sock, SOMAXCONN ) == SOCKET_ERROR ) {
        printf( "listen failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    return 0;
}

SOCKET sock_accept(SOCKET sock){
    SOCKET client = INVALID_SOCKET;

    client = accept(sock, NULL, NULL);
    if (client == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    printf("[+] new connection\n");

    return client;
}


int sock_recv(SOCKET sock, char* buffer, int buff_size) {
    int iResult;

    if (buff_size <= 0) {
        buff_size = DEFAULT_BUFLEN;
    }

    iResult = recv(sock, buffer, buff_size, 0);
    if (iResult > 0) {
        // printf("Bytes received: %d\n", iResult);
        buffer[iResult] = '\0';  // Null-terminate the received data
        return iResult;

    } else if (iResult == 0) {
        printf("Connection closing...\n");
        return 0;
    } else {
        printf("recv failed: %d\n", WSAGetLastError());
        return -1;
    }
}
