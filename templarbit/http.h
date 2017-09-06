#ifndef _HTTP_H_
#define _HTTP_H_

#include "list.h"

/**
 * Supported HTTP methods
 */
typedef enum {GET, POST} http_method_t;

/**
 * Describes list of HTTP headers
 * that could be passed to remote server
 */
struct http_header
{
   struct http_header* next;
   char* header;
};

typedef struct http_header http_header_t;

/**
 * Describes HTTP request
 * that could be passed to remote server
 */ 
struct http_request
{
   /**
    * Remote URL to send request to
    */
   char* url;

   /**
    * Request body
    * supported for POST method only. Optional.
    */
   char* request_body;

   /**
    * HTTP method
    */
   http_method_t method;

   /**
    * Optional list of headers to send
    * to remote server
    */
   http_header_t* headers;

   /**
    * Request connect timeout
    */
   unsigned int connect_timeout;

   /**
    * Timeout for response retrieval
    */
   unsigned int response_timeout;
};

typedef struct http_request http_request_t;

/**
 * Describes HTTP response state
 */ 
struct http_response
{
   /**
    * Received response from server
    */
   char* response_body;
   size_t response_body_size;

   /**
    * HTTP response code (i.e. 200/404/...)
    */
   int response_code;

   /**
    * Error frim underlying library
    * must not be freed using 'free' function
    */
   const char* curl_error;

   /**
    * Error code from underlying library used for 
    * making HTTP calls
    */
   int curl_code;
};

typedef struct http_response http_response_t;

http_response_t* http_post(http_request_t* request);
http_request_t* make_http_request(char* url, char* request_body, http_method_t method, http_header_t* headers, unsigned int connect_timeout, unsigned int response_timeout);
void header_append_node(http_header_t** root, http_header_t* node);
http_header_t* header_append_node_n(http_header_t** root, char* header);
void free_http_request(http_request_t* request);
void free_http_response(http_response_t* response);
void free_http_headers(http_header_t* headers);

#endif
