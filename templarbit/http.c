#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "http.h"

static size_t http_write_reponse(void *contents, size_t size, size_t nmemb,
      void *userp)
{
   size_t realsize = size * nmemb;
   http_response_t *response = (http_response_t*) userp;

   response->response_body = realloc(response->response_body,
         response->response_body_size + realsize + 1);
   if (response->response_body == NULL)
   {
      return 0;
   }

   memcpy(&(response->response_body[response->response_body_size]), contents,
         realsize);
   response->response_body_size += realsize;
   response->response_body[response->response_body_size] = 0;

   return realsize;
}

http_request_t* make_http_request(char* url, char* request_body,
      http_method_t method, http_header_t* headers,
      unsigned int connect_timeout, unsigned int response_timeout)
{
   http_request_t* request = (http_request_t*) calloc(1,
         sizeof(http_request_t));

   request->url = strdup(url);
   request->request_body = strdup(request_body);
   request->method = method;
   request->headers = headers;
   request->connect_timeout = connect_timeout;
   request->response_timeout = response_timeout;

   return request;
}

void free_http_request(http_request_t* request)
{
   free(request->url);
   free(request->request_body);
   free_http_headers(request->headers);
   free(request);
}

void free_http_response(http_response_t* response)
{
   free(response->response_body);
   free(response);
}

static int free_http_header(struct list_node* raw_node, void* uptr)
{
   http_header_t* node = (http_header_t*) raw_node;
   free(node->header);
   free(node);

   return 0;
}

void free_http_headers(http_header_t* headers)
{
   iterate_list((struct list_node*) headers, &free_http_header, NULL);
}

void header_append_node(http_header_t** root, http_header_t* node)
{
   append_node((struct list_node**) root, (struct list_node*) node);
}

http_header_t* header_append_node_n(http_header_t** root, char* header)
{
   http_header_t* node = calloc(1, sizeof(http_header_t));
   node->header = strdup(header);
   node->next = NULL;

   header_append_node(root, node);
   return node;
}

static int setup_curl_headers(struct list_node* raw_node, void* curl_slist)
{
   http_header_t* node = (http_header_t*) raw_node;

   struct curl_slist **headers = (struct curl_slist**) curl_slist;
   *headers = curl_slist_append(*headers, node->header);

   return 0;
}

http_response_t* http_post(http_request_t* request)
{
   CURL *curl = curl_easy_init();

   if (curl)
   {
      http_response_t* response = malloc(sizeof(http_response_t));
      memset(response, 0, sizeof(http_response_t));

      // setting initial buffer size for the reponse
      // it will be re-allocated upon response retrieval by http_write_reponse function
      response->response_body = (char *) calloc(1, sizeof(char));
      response->response_body_size = 0;

      curl_easy_setopt(curl, CURLOPT_URL, request->url);

      switch (request->method)
      {
         case GET:
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            break;
         case POST:
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->request_body);
            break;
      }

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_reponse);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * ) response);
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-nginx-agent/1.0");
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
      curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, request->connect_timeout);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, request->response_timeout);

      // setup curl headers
      struct curl_slist *headers = NULL;
      if (request->headers)
      {
         iterate_list((struct list_node*) request->headers, &setup_curl_headers,
               &headers);
         curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      }

      // perform request
      response->curl_code = curl_easy_perform(curl);

      // get CURL error text
      if (response->curl_code != CURLE_OK)
      {
         response->curl_error = curl_easy_strerror(response->curl_code);
      }

      // getting HTTP status code
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
            &(response->response_code));

      if (headers)
      {
         curl_slist_free_all(headers);
      }

      curl_easy_cleanup(curl);

      return response;
   }

   return NULL;
}
