#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#ifndef DEFAULT_BUFLEN
    #define DEFAULT_BUFLEN 1024
#endif

// create a new TCP/IP server socket
// @param port the port of the server socket
// @returns a new server socket (`INVALID_SOCKET` if there is an error)
SOCKET create_sock(char* port);

// destroy a socket
// @param sock the server socket
// @returns a non zero value to indicate an error
int sock_destroy(SOCKET sock);

// listen on a socket for incomming connections
// @param sock the server socket
// @returns a non zero value to indicate an error
int sock_listen(SOCKET sock);

// accept a new connection from a client
// returns the client socket
// @param sock the server socket
// @returns the client socket (`INVALID_SOCKET` if there is an error)
SOCKET sock_accept(SOCKET sock);

// accept data from a client
// @param sock the server socket
// @param buffer the buffer where the received data will be saved
// @param buf_size size of the buffer in bytes (if <=0 DEFAULT_BUFLEN will be used)
// @returns the data received by the client socket (you should free it).
//
// NULL stands for an error, or that the client has closed the connection
int sock_recv(SOCKET sock, char* buffer, int buff_size);

