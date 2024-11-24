
#include "requests.h"
#include "logger.h"
#include <time.h>
#include <unistd.h>


Result ConnResult;


// crate a new Header struct
Header create_header(string name, string value){
    Header header;
    header.name = name;
    header.value = value;
    return header;
}


string get_datetime_header(){
    int date_buffer_size = 100;
    string datetime = malloc(date_buffer_size);
    if (!datetime) return "";

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(datetime, date_buffer_size, "Date: %d-%02d-%02d %02d:%02d:%02d",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    return datetime;
}


Response error_response(int errorCode) {
    Response res;
    char contentLen[32];

    res.http_version = strdup("HTTP/1.1");
    res.status = strdup(status_text(errorCode));
    
    // Allocate body buffer
    res.body = (char*)malloc(256);
    if (res.body) {
        snprintf(res.body, 256, "<html><head><title>%s</title></head>"
                "<body><center><h1>%s</h1></center></body></html>", 
                res.status, res.status);
    }

    // Allocate and set headers properly
    res.headers = (char**)malloc(5 * sizeof(char*));  // Space for 4 headers + NULL
    if (res.headers) {
        res.headers[0] = strdup("Content-Type: text/html; charset=UTF-8");
        snprintf(contentLen, sizeof(contentLen), "Content-Length: %zu", 
                res.body ? strlen(res.body) : 0);
        res.headers[1] = strdup(contentLen);
        res.headers[2] = get_datetime_header();  // Already allocated
        res.headers[3] = strdup("Connection: Keep-Alive");
        res.headers[4] = NULL;  // NULL terminator
    }

    return res;
}


Response process_request(Request *request, http_server *server){
    string method = request->method;

    if (strcasecmp(method, "GET") == 0)
        return get_request(*request, server);
    log("unsupported method: %s\n", method);
    return error_response(STATUS_METHOD_NOT_ALLOWED);
}


Request* parse_request(string request){
    Request* req = (Request*)malloc(sizeof(Request));
    if (!req) {
        perror("Failed to allocate memory for Request");
        return NULL;
    }

    // Initialize the request struct
    req->headers = NULL;
    req->body = NULL;

    // Split the request into lines
    char* request_copy = strdup(request);           // Duplicate the request string
    char* line = strtok(request_copy, "\r\n");

    // Parse the request line (first line)
    if (line) {
        char* method = strtok(line, " ");
        char* path_str = strtok(NULL, " ");
        char* http_version = strtok(NULL, " ");

        if (method && path_str && http_version) {
            req->method = strdup(method);
            req->http_version = strdup(http_version);

            // Parse the path into location, path, search, and fragment
            req->path.location = strdup(path_str);
            req->path.path = strdup(path_str);      // Default to the full path
            req->path.search = NULL;                // Initialize search parameters
            req->path.fragment = NULL;              // Initialize fragment

            // Check for search parameters and fragment
            char* search_start = strchr(path_str, '?');
            char* fragment_start = strchr(path_str, '#');

            if (search_start) {
                *search_start = '\0';                           // Split the path
                req->path.path = strdup(path_str);              // Update path
                req->path.search = strdup(search_start + 1);    // Get search parameters
            }

            if (fragment_start) {
                *fragment_start = '\0';                             // Split the path
                req->path.path = strdup(path_str);                  // Update path
                req->path.fragment = strdup(fragment_start + 1);    // Get fragment
            }
        }
    }

    // Parse headers
    size_t header_count = 0;
    size_t header_capacity = 4; // Initial capacity for headers
    req->headers = (Header*)malloc(header_capacity * sizeof(Header));

    while ((line = strtok(NULL, "\r\n")) && strlen(line) > 0) {

        // expand header field
        if (header_count >= header_capacity) {
            header_capacity *= 2;
            req->headers = (Header*)realloc(req->headers, header_capacity * sizeof(Header));
        }

        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0'; // Split the key and value
            req->headers[header_count].name = strdup(line);
            req->headers[header_count].value = strdup(colon + 1); // Skip the colon
            header_count++;

            // check the type of connection
            if (stricmp(req->headers[header_count].name, "Connection") == 0){
                // 1=true, 0=false (the following line converts a 1 in 0 and 0 in 1)
                ConnResult.close = 1 - stricmp(req->headers[header_count].value, "close");
            }

        }
    }

    // Handle the body (if present, ex. POST requests)
    if (line) {
        req->body = strdup(line);
    }

    // Clean up
    free(request_copy);
    return req;
}


void free_if_not_null(void* ptr){
    if(ptr)
        free(ptr);
}


void free_request(Request* req) {
    if (!req) return;
    
    free_if_not_null(req->method);
    free_if_not_null(req->http_version);
    free_if_not_null(req->path.location);
    free_if_not_null(req->path.path);
    free_if_not_null(req->path.search);
    free_if_not_null(req->path.fragment);

    if (req->headers){
        for (size_t i = 0; req->headers && req->headers[i].name; i++) {
            free_if_not_null(req->headers[i].name);
            free_if_not_null(req->headers[i].value);
        }
        free(req->headers);
    }
    free_if_not_null(req->body);
    free(req);

}


void free_response(Response* res) {
    if (!res) return;

    // Only free http_version if it was dynamically allocated
    if (res->http_version && strcmp(res->http_version, "HTTP/1.1") != 0) {
        free(res->http_version);
    }

    if (res->headers){
        for (size_t i = 0; res->headers && res->headers[i]; i++) {
            free_if_not_null(res->headers[i]);
        }
        free(res->headers);
    }

    free_if_not_null(res->body);
}


string serialize_response(Response response) {
    // Calculate the total size needed for headers
    size_t headers_size = strlen(response.http_version) + strlen(response.status) + 4; // +4 for " ", \r\n
    
    // Calculate size needed for headers
    for (size_t i = 0; response.headers && response.headers[i]; i++) {
        headers_size += strlen(response.headers[i]) + 2; // +2 for \r\n
    }
    headers_size += 2; // For the extra \r\n after headers

    // If we have a body, get its length from the Content-Length header
    size_t body_size = 0;
    if (response.body) {
        for (size_t i = 0; response.headers && response.headers[i]; i++) {
            if (strncmp(response.headers[i], "Content-Length: ", 16) == 0) {
                body_size = atol(response.headers[i] + 16);
                break;
            }
        }
    }

    // Allocate memory for the complete response
    size_t total_size = headers_size + body_size;
    char* res = malloc(total_size);
    if (!res) {
        perror("Failed to allocate memory for response");
        return NULL;
    }

    // Write headers
    char* current = res;
    int written = snprintf(current, headers_size, "%s %s\r\n", 
                          response.http_version, response.status);
    current += written;

    // Write each header
    for (size_t i = 0; response.headers && response.headers[i]; i++) {
        written = snprintf(current, headers_size - (current - res), 
                          "%s\r\n", response.headers[i]);
        current += written;
    }

    // Add the blank line after headers
    written = snprintf(current, headers_size - (current - res), "\r\n");
    current += written;

    // Copy the body if it exists
    if (response.body && body_size > 0) {
        memcpy(current, response.body, body_size);
    }

    return res;
}


string handle_request(string request, http_server *server){
    Request *req = parse_request(request);
    Response res;
    
    if (req == NULL){
        res = error_response(STATUS_BAD_REQUEST);
        free_request(req);
        printf("[???] [%s]\n", res.status);
    } else {
        res = process_request(req, server);
        printf("[%s] [%s] %s\n", req->method, res.status, req->path.location);
        // free_request(req);
    }

    string sres = serialize_response(res); 
    free_response(&res);
    return sres;
}


BOOL is_path_safe(const string path) {
    // Check for NULL or empty path
    if (!path || !*path) return FALSE;
    
    // Check for ".." sequences that could lead to directory traversal
    const char* ptr = path;
    while (*ptr) {
        if (ptr[0] == '.' && ptr[1] == '.' && 
           (ptr[2] == '/' || ptr[2] == '\\' || ptr[2] == '\0')) {
            return FALSE;
        }
        ptr++;
    }
    
    // Check for absolute paths or drive letters (Windows)
    if (path[0] == '/' || path[0] == '\\' || 
       (isalpha(path[0]) && path[1] == ':')) {
        return FALSE;
    }
    
    return TRUE;
}


// Remove leading slashes and set default page
void normalize_path(Request* request) {
    if (!request || !request->path.path) return;
    
    // Skip all leading slashes
    const char* path = request->path.path;
    while (*path == '/') path++;
    
    // If path is empty after removing slashes, set to index.html
    if (*path == '\0') {
        free(request->path.path);  // Free the old path
        request->path.path = strdup("index.html");
        return;
    }
    
    // Otherwise, create new string without leading slashes
    char* new_path = strdup(path);
    if (!new_path) return;

    if (is_path_safe(new_path) == FALSE) {
        free(new_path);
        free(request->path.path);
        request->path.path = strdup("index.html");  // Default to index.html on unsafe paths
        return;
    }

    free(request->path.path);  // Free the old path
    request->path.path = new_path;
}


// Helper function to determine the Content-Type header
const string get_content_type(const string path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    ext++; // Skip the dot

    if (strcasecmp(ext, "html") == 0 || strcasecmp(ext, "htm") == 0) {
        return "text/html; charset=UTF-8";
    } else if (strcasecmp(ext, "css") == 0) {
        return "text/css";
    } else if (strcasecmp(ext, "js") == 0) {
        return "application/javascript";
    } else if (strcasecmp(ext, "png") == 0) {
        return "image/png";
    } else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) {
        return "image/jpeg";
    } else if (strcasecmp(ext, "gif") == 0) {
        return "image/gif";
    } else if (strcasecmp(ext, "svg") == 0) {
        return "image/svg+xml";
    } else if (strcasecmp(ext, "ico") == 0) {
        return "image/x-icon";
    }
    
    return "application/octet-stream";
}


Response get_request(Request request, http_server* server) {
    Response res;
    memset(&res, 0, sizeof(Response));

    res.http_version = strdup("HTTP/1.1");

    // handle path sanitization
    normalize_path(&request);

    FILE *file = fopen(request.path.path, "rb");  // Note: "rb" for binary reading
    if (file == NULL) {
        return error_response(STATUS_NOT_FOUND);
    }

    // read the whole file
    fseek(file, 0L, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate exactly file_size bytes (no null terminator needed for binary data)
    res.body = malloc(file_size);
    if (!res.body) {
        perror("Failed to allocate memory");
        fclose(file);
        return error_response(STATUS_INTERNAL_SERVER_ERROR);
    }

    // Read the file into the buffer
    size_t bytes_read = fread(res.body, 1, file_size, file);
    if (bytes_read != file_size) {
        perror("Failed to read the complete file");
        free(res.body);
        fclose(file);
        return error_response(STATUS_INTERNAL_SERVER_ERROR);
    }
    fclose(file);

    // assemble the response
    res.headers = malloc(6 * sizeof(char*));
    if (!res.headers) {
        free(res.body);
        return error_response(STATUS_INTERNAL_SERVER_ERROR);
    }

    // determine the content-type header based on the type of file
    const char* content_type = get_content_type(request.path.path);

    char content_length[32], stream_header[64], content_type_header[100];
    snprintf(content_length, sizeof(content_length), "Content-Length: %lld", bytes_read);
    snprintf(stream_header, sizeof(stream_header), "Server: %s", server->serverName);
    snprintf(content_type_header, sizeof(content_type_header), "Content-Type: %s", content_type);

    res.status = strdup(status_text(STATUS_OK));
    res.headers[0] = strdup(content_type_header);
    res.headers[1] = get_datetime_header();
    res.headers[2] = strdup(stream_header);
    res.headers[3] = strdup(content_length);
    res.headers[4] = strdup("Connection: keep-alive"); // in http/1.1 this is the default
    res.headers[5] = NULL;

    return res;
}


Result get_last_connection_results(){
    return ConnResult;
}


string status_text(int code) {
	switch (code) {
        case STATUS_CONTINUE:                           return "100 Continue";
        case STATUS_SWITCHING_PROTOCOLS:                return "101 Switching Protocols";
        case STATUS_PROCESSING:                         return "102 Processing";
        case STATUS_EARLY_HINTS:                        return "103 Early Hints";
        case STATUS_OK:                                 return "200 OK";
        case STATUS_CREATED:                            return "201 Created";
        case STATUS_ACCEPTED:                           return "202 Accepted";
        case STATUS_NON_AUTHORITATIVE_INFO:             return "203 Non-Authoritative Information";
        case STATUS_NO_CONTENT:                         return "204 No Content";
        case STATUS_RESET_CONTENT:                      return "205 Reset Content";
        case STATUS_PARTIAL_CONTENT:                    return "206 Partial Content";
        case STATUS_MULTI_STATUS:                       return "207 Multi-Status";
        case STATUS_ALREADY_REPORTED:                   return "208 Already Reported";
        case STATUS_IM_USED:                            return "226 IM Used";
        case STATUS_MULTIPLE_CHOICES:                   return "300 Multiple Choices";
        case STATUS_MOVED_PERMANENTLY:                  return "301 Moved Permanently";
        case STATUS_FOUND:                              return "302 Found";
        case STATUS_SEE_OTHER:                          return "303 See Other";
        case STATUS_NOT_MODIFIED:                       return "304 Not Modified";
        case STATUS_USE_PROXY:                          return "305 Use Proxy";
        case STATUS_TEMPORARY_REDIRECT:                 return "307 Temporary Redirect";
        case STATUS_PERMANENT_REDIRECT:                 return "308 Permanent Redirect";
        case STATUS_BAD_REQUEST:                        return "400 Bad Request";
        case STATUS_UNAUTHORIZED:                       return "401 Unauthorized";
        case STATUS_PAYMENT_REQUIRED:                   return "402 Payment Required";
        case STATUS_FORBIDDEN:                          return "403 Forbidden";
        case STATUS_NOT_FOUND:                          return "404 Not Found";
        case STATUS_METHOD_NOT_ALLOWED:                 return "405 Method Not Allowed";
        case STATUS_NOT_ACCEPTABLE:                     return "406 Not Acceptable";
        case STATUS_PROXY_AUTH_REQUIRED:                return "407 Proxy Authentication Required";
        case STATUS_REQUEST_TIMEOUT:                    return "408 Request Timeout";
        case STATUS_CONFLICT:                           return "409 Conflict";
        case STATUS_GONE:                               return "410 Gone";
        case STATUS_LENGTH_REQUIRED:                    return "411 Length Required";
        case STATUS_PRECONDITION_FAILED:                return "412 Precondition Failed";
        case STATUS_REQUEST_ENTITY_TOO_LARGE:           return "413 Request Entity Too Large";
        case STATUS_REQUEST_URI_TOO_LONG:               return "414 Request URI Too Long";
        case STATUS_UNSUPPORTED_MEDIA_TYPE:             return "415 Unsupported Media Type";
        case STATUS_REQUESTED_RANGE_NOT_SATISFIABLE:    return "416 Requested Range Not Satisfiable";
        case STATUS_EXPECTATION_FAILED:                 return "417 Expectation Failed";
        case STATUS_TEAPOT:                             return "418 I'm a teapot";
        case STATUS_MISDIRECTED_REQUEST:                return "421 Misdirected Request";
        case STATUS_UNPROCESSABLE_ENTITY:               return "422 Unprocessable Entity";
        case STATUS_LOCKED:                             return "423 Locked";
        case STATUS_FAILED_DEPENDENCY:                  return "424 Failed Dependency";
        case STATUS_TOO_EARLY:                          return "425 Too Early";
        case STATUS_UPGRADE_REQUIRED:                   return "426 Upgrade Required";
        case STATUS_PRECONDITION_REQUIRED:              return "428 Precondition Required";
        case STATUS_TOO_MANY_REQUESTS:                  return "429 Too Many Requests";
        case STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE:    return "431 Request Header Fields Too Large";
        case STATUS_UNAVAILABLE_FOR_LEGAL_REASONS:      return "451 Unavailable For Legal Reasons";
        case STATUS_INTERNAL_SERVER_ERROR:              return "500 Internal Server Error";
        case STATUS_NOT_IMPLEMENTED:                    return "501 Not Implemented";
        case STATUS_BAD_GATEWAY:                        return "502 Bad Gateway";
        case STATUS_SERVICE_UNAVAILABLE:                return "503 Service Unavailable";
        case STATUS_GATEWAY_TIMEOUT:                    return "504 Gateway Timeout";
        case STATUS_HTTP_VERSION_NOT_SUPPORTED:         return "505 HTTP Version Not Supported";
        case STATUS_VARIANT_ALSO_NEGOTIATES:            return "506 Variant Also Negotiates";
        case STATUS_INSUFFICIENT_STORAGE:               return "507 Insufficient Storage";
        case STATUS_LOOP_DETECTED:                      return "508 Loop Detected";
        case STATUS_NOT_EXTENDED:                       return "510 Not Extended";
        case STATUS_NETWORK_AUTHENTICATION_REQUIRED:    return "511 Network Authentication Required";
	}
    return "";
}

