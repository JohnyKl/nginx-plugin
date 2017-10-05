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

/**
 * Makes HTTP POST request
 *
 * @param request pointer to http_request_t
 * 
 * @return pointer to http_response_t
 */
http_response_t* http_post(http_request_t* request);

/**
 * Constructs http_request_t by given set of parameters
 *
 * @param url URL to make request to
 * @param request_body request body, may be NULL if method is GET
 * @param method HTTP method, refer to http_method_t enum
 * @param headers linked list of headers, may be NULL
 * @param connect_timeout HTTP connection timeout
 * @param response_timeout HTTP response timeout
 * 
 * @return pointer to http_request_t
 */
http_request_t* make_http_request(char* url, char* request_body, http_method_t method, http_header_t* headers, unsigned int connect_timeout, unsigned int response_timeout);

/**
 * Append HTTP header to the list
 *
 * @param root root node of the linked list, may be NULL, in this case new list will get created
 * @param node unique handler identifier
 * 
 * @return pointer to the new header node
 */
void header_append_node(http_header_t** root, http_header_t* node);

/**
 * Creates node using hiven header and appends it to the list with root element which points by 'root' argument
 *
 * @param root root node of the linked list, may be NULL, in this case new list will get created
 * @param header header in horm of 'HeaderName: HeaderValue'
 *
 * @return
 */
http_header_t* header_append_node_n(http_header_t** root, char* header);

/**
 * Free memory allocated by HTTP request struct represented by http_request_t
 *
 * @param request pointer to request to be freed
 */
void free_http_request(http_request_t* request);

/**
 * Free memory allocated by HTTP response represented by http_response_t
 *
 * @param request pointer to response to be freed
 */
void free_http_response(http_response_t* response);

/**
 * Free memory allocated by list of HTTP headers represented by http_header_t
 *
 * @param request pointer to headers to be freed
 */
void free_http_headers(http_header_t* headers);

#endif
