#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "http.h"

struct http_response {
  char *data;
  size_t size;
};

static size_t
http_write_reponse(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct http_response *response = (struct http_response *)userp;

  response->data = realloc(response->data, response->size + realsize + 1);
  if (response->data == NULL) {
     return 0;
  }

  memcpy(&(response->data[response->size]), contents, realsize);
  response->size += realsize;
  response->data[response->size] = 0;

  return realsize;
}

char* http_post(char* url, char* request_body, long* response_code, long connect_timeout, long response_timeout)
{
    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl)
    {
       struct http_response chunk;
       chunk.data = malloc(1);
       chunk.size = 0;

       curl_easy_setopt(curl, CURLOPT_URL, url);
       curl_easy_setopt(curl, CURLOPT_POST, 1L);
       curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);
       curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_reponse);
       curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
       curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
       curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
       curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
       curl_easy_setopt(curl, CURLOPT_TIMEOUT, response_timeout);

       struct curl_slist *headers = NULL;
       headers = curl_slist_append(headers, "Content-Type: application/json");
       curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

       res = curl_easy_perform(curl);

       curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, response_code);

       if (res != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

       curl_easy_cleanup(curl);

       return chunk.data;
    }
    else
    {
       *response_code = -1;
       return NULL;
    }
}
