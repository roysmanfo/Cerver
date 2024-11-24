#pragma once

#include "structs.h"

enum {
    STATUS_CONTINUE                         = 100,
	STATUS_SWITCHING_PROTOCOLS              = 101,
	STATUS_PROCESSING                       = 102,
	STATUS_EARLY_HINTS                      = 103,

	STATUS_OK                               = 200,
	STATUS_CREATED                          = 201,
	STATUS_ACCEPTED                         = 202,
	STATUS_NON_AUTHORITATIVE_INFO           = 203,
	STATUS_NO_CONTENT                       = 204,
	STATUS_RESET_CONTENT                    = 205,
	STATUS_PARTIAL_CONTENT                  = 206,
	STATUS_MULTI_STATUS                     = 207,
	STATUS_ALREADY_REPORTED                 = 208,
	STATUS_IM_USED                          = 226,

	STATUS_MULTIPLE_CHOICES                 = 300,
	STATUS_MOVED_PERMANENTLY                = 301,
	STATUS_FOUND                            = 302,
	STATUS_SEE_OTHER                        = 303,
	STATUS_NOT_MODIFIED                     = 304,
	STATUS_USE_PROXY                        = 305,
	// _                                       = 306, (unused)
	STATUS_TEMPORARY_REDIRECT               = 307,
	STATUS_PERMANENT_REDIRECT               = 308,

	STATUS_BAD_REQUEST                      = 400,
	STATUS_UNAUTHORIZED                     = 401,
	STATUS_PAYMENT_REQUIRED                 = 402,
	STATUS_FORBIDDEN                        = 403,
	STATUS_NOT_FOUND                        = 404,
	STATUS_METHOD_NOT_ALLOWED               = 405,
	STATUS_NOT_ACCEPTABLE                   = 406,
	STATUS_PROXY_AUTH_REQUIRED              = 407,
	STATUS_REQUEST_TIMEOUT                  = 408,
	STATUS_CONFLICT                         = 409,
	STATUS_GONE                             = 410,
	STATUS_LENGTH_REQUIRED                  = 411,
	STATUS_PRECONDITION_FAILED              = 412,
	STATUS_REQUEST_ENTITY_TOO_LARGE         = 413,
	STATUS_REQUEST_URI_TOO_LONG             = 414,
	STATUS_UNSUPPORTED_MEDIA_TYPE           = 415,
	STATUS_REQUESTED_RANGE_NOT_SATISFIABLE  = 416,
	STATUS_EXPECTATION_FAILED               = 417,
	STATUS_TEAPOT                           = 418, // (unused)
	STATUS_MISDIRECTED_REQUEST              = 421,
	STATUS_UNPROCESSABLE_ENTITY             = 422,
	STATUS_LOCKED                           = 423,
	STATUS_FAILED_DEPENDENCY                = 424,
	STATUS_TOO_EARLY                        = 425,
	STATUS_UPGRADE_REQUIRED                 = 426,
	STATUS_PRECONDITION_REQUIRED            = 428,
	STATUS_TOO_MANY_REQUESTS                = 429,
	STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE  = 431,
	STATUS_UNAVAILABLE_FOR_LEGAL_REASONS    = 451,

	STATUS_INTERNAL_SERVER_ERROR            = 500,
	STATUS_NOT_IMPLEMENTED                  = 501,
	STATUS_BAD_GATEWAY                      = 502,
	STATUS_SERVICE_UNAVAILABLE              = 503,
	STATUS_GATEWAY_TIMEOUT                  = 504,
	STATUS_HTTP_VERSION_NOT_SUPPORTED       = 505,
	STATUS_VARIANT_ALSO_NEGOTIATES          = 506,
	STATUS_INSUFFICIENT_STORAGE             = 507,
	STATUS_LOOP_DETECTED                    = 508,
	STATUS_NOT_EXTENDED                     = 510,
	STATUS_NETWORK_AUTHENTICATION_REQUIRED  = 511
};

// @param code the status code
// @returns the reason corresponding to a status code 
string status_text(int code);


// creates a response string that can be sent over the network
// @param response the response struct that needs to be serialized  
// @returns a response string that is ready to be sent over the network  
string serialize_response(Response response);


// handle the request and provide a response to the client
// @param request the request from the client  
// @param server a pointer to the server instance (contains some infomation)   
// @returns a response that can be sent over the network
string handle_request(string request, http_server *server);


// creates a Response struct that can be processed by `serialize_response()`
// before being sent to the client
// @param request the response struct that needs to be serialized  
// @param server a pointer to the server instance (contains some infomation)   
// @returns a response struct that should be passed to `serialize_response()`
Response process_request(Request *request, http_server *server);

// Remove leading slashes and set default page
// (this operation is performed in-place, therefore there is no return)
// @param request a poiter to the request to normalize
void normalize_path(Request* request);

// handler for get requests
Response get_request(Request request, http_server *server);

// get the results of the last handled connection
Result get_last_connection_results();
