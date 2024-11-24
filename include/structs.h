#pragma once 
#include "sock.h"

typedef char *string;

typedef struct {
    // the winsock where the http server will run
    SOCKET socket;

    // the maximum number of connections thar the server will be able to handle
    DWORD maxConnections;

    // an array of handles to all active connections
    HANDLE *hThreads; 

    // an array of thread IDs to all active connections
    DWORD *hThreadIds; 

    // number of active connections (clients)
    DWORD numConnections;
    
    // the http version of the server
    string version;
    
    // for example Apache/Nginx/ISS/... 
    // defaults to Cerver (C server) 
    string serverName;
} http_server;

typedef struct {
    // the complete path form the request (host:port/path?search#fragment)
    string location;
    
    // the page path (/path?search#fragment)
    string path;
    
    // the search parameters (?search)
    string search;

    // the page fragment (#fragment)
    string fragment;
} Path;

typedef struct{
    // complete name of the header
    string name;

    // value of the header
    string value;
} Header;

typedef struct {
    // method of the request
    string method;

    // http version of the request
    string http_version;

    // request path (ex. /profile/edit)
    Path path;

    // request headers
    Header *headers;
    
    // request body
    string body;
} Request;

typedef struct {
    // http version of the response
    string http_version;

    // returned status code + message
    string status;    

    // response path (ex. /profile/edit)
    Path path;

    // response headers
    string *headers;
    
    // response body
    string body;
} Response;


typedef struct {
    // whether the client asked to close the connection
    // 1 = true, 0 = false (so that it works in an if/else block)
    short close; 

    

} Result;
