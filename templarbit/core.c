#include "core.h"
#include "request.h"
#include "http.h"
#include "handler.h"
#include "../vendor/jansson/src/jansson.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>

#include <curl/curl.h>

#ifndef _TESTS

#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static ngx_int_t ngx_init_zone(ngx_shm_zone_t *shm_zone, void *data);

#else

struct ngx_shm_zone
{
   int (*init)(void*, void*);
   void* data;
   void* shm;
};

typedef struct ngx_shm_zone ngx_shm_zone_t;

#define ngx_log_stderr(level, message, ...) printf(message, __VA_ARGS__);

#define ngx_init_zone 0

ngx_shm_zone_t* mock_ngx_shared_memory_add(void *cf, void *name, size_t size,
      void *module);

typedef char* ngx_str_t;
#define ngx_str_set(dest, source) { }
#define ngx_shared_memory_add(cf, name, size, module) mock_ngx_shared_memory_add(cf, name, size, module)
#define ngx_shmtx_lock(mutex) { }
#define ngx_shmtx_unlock(mutex) { }

#endif

int poll_api_impl(struct handler_node *handler, char* url, char* request_body,
      int timeout)
{
   json_error_t json_error;
   json_t *root_node = NULL;
   int result = REQ_OK;
   const char *new_csp, *new_csp_ro;
   char *curr_csp, *curr_csp_ro;
   json_t *raw_csp, *raw_csp_ro;
   int csp_not_equals = 0;
   int csp_ro_not_equals = 0;

   // constructing request struct

   http_header_t* headers = NULL;
   header_append_node_n(&headers, "Content-Type: application/json");
   http_request_t* request = make_http_request(url, request_body, POST, headers,
         timeout, timeout);

   // making request to templarbit server
   http_response_t* response = http_post(request);

   if (!response || !response->response_body
         || response->curl_code == CURLE_OPERATION_TIMEDOUT)
   {
      result = REQ_NO_RESPONSE;
      goto finalize_request;
   }

   if (response->response_code != 200)
   {
      result = REQ_FAILED;
      goto finalize_request;
   }

   root_node = json_loads(response->response_body, 0, &json_error);
   if (!root_node)
   {
      result = REQ_MALFORMED_RESPONSE;
      goto finalize_request;
   }

   raw_csp = json_object_get(root_node, "csp");
   raw_csp_ro = json_object_get(root_node, "csp_report_only");

   if (!json_is_string(raw_csp) && !json_is_string(raw_csp_ro))
   {
      result = REQ_INVALID_RESPONSE;
      goto finalize_request;
   }

   new_csp = json_string_value(raw_csp);
   new_csp_ro = json_string_value(raw_csp_ro);

   while (!handler->csp_headers)
   {
      handler->csp_headers = *((struct headers**) ((ngx_shm_zone_t *) handler
            ->shm_zone)->data);
      sleep(timeout);
   }

   curr_csp = handler->csp_headers->csp;
   curr_csp_ro = handler->csp_headers->csp_ro;

   csp_not_equals = new_csp && strcmp(curr_csp, new_csp);
   csp_ro_not_equals = new_csp_ro && strcmp(curr_csp_ro, new_csp_ro);

   if (csp_not_equals // CSP are inequal
   || csp_ro_not_equals) // CSP RO are inequal
   {
      // incrementing version number
      handler->csp_headers->version = handler->csp_headers->version + 1;

      // locking shared memory segment for update operation
      ngx_shmtx_lock(_NGX_SHM_MUTEX(handler->shm));

      if (new_csp)
      {
         memset(curr_csp, 0, CSP_HEADER_SIZE);
         strcpy(curr_csp, new_csp);
      }

      if (new_csp_ro)
      {
         memset(curr_csp_ro, 0, CSP_HEADER_SIZE);
         strcpy(curr_csp_ro, new_csp_ro);
      }

      // releasing shared memory segment lock
      ngx_shmtx_unlock(_NGX_SHM_MUTEX(handler->shm));

      ngx_log_stderr(0,
            "Templarbit: New headers received. Content-Security-Policy: '%s', Content-Security-Policy-Report-Only: '%s'",
            (new_csp ? new_csp : ""), (new_csp_ro ? new_csp_ro : ""));
   }

   finalize_request: json_decref(root_node);
   free_http_request(request);
   free_http_response(response);

   return result;
}

int process_server_instance(void *cf, struct handler_node **handlers,
      char* token, char *property_id, void* server_config,
      void* ngx_http_templarbit_csp_module)
{
   struct handler_node *handler; /* templarbit server handler node */

   ngx_str_t name;
   ngx_str_set(&name, property_id);

   // checking if we have already handler/poller for the specific token
   handler = handler_find_node(*handlers, token);
   if (handler)
   {
      return HANDLER_EXISTS;
   }

   // preparing request body
   json_t *request = json_pack("{ssss}", "token", token, "property_id",
         property_id);

   handler = handler_append_node_n(handlers, token);
   handler->request_body = json_dumps(request, JSON_COMPACT);
   handler->clsv = server_config;
   handler->csp_headers = NULL;

   // allocating shared memory segment for storing CSP header
   /* add an entry for CSP_SIZE shared zone */
   int size = (CSP_SIZE) * 4;
   handler->shm_zone = ngx_shared_memory_add(cf, &name, size, // + sizeof(ngx_slab_pool_t),
         ngx_http_templarbit_csp_module);
   if (handler->shm_zone == NULL)
   {
      return HANDLER_SHM_INIT_FAILED;
   }

   /* register init callback and context */
   ((ngx_shm_zone_t *) handler->shm_zone)->init = ngx_init_zone;
   ((ngx_shm_zone_t *) handler->shm_zone)->data = &handler->csp_headers;
   handler->shm = &((ngx_shm_zone_t *) handler->shm_zone)->shm;

   return HANDLER_STARTED;
}

#ifndef _TESTS
static ngx_int_t ngx_init_zone(ngx_shm_zone_t *shm_zone, void *data)
{
   struct headers *ctx;
   ngx_slab_pool_t *shpool;

   if (data)
   {
      /* reusing a shared zone from old cycle */
      shm_zone->data = data;
      return NGX_OK;
   }

   shpool = (ngx_slab_pool_t*) shm_zone->shm.addr;

   /* initialize shared zone */
   ctx = ngx_slab_alloc(shpool, sizeof(struct headers));
   if (ctx == NULL)
   {
      return NGX_ERROR;
   }

   *((struct headers**) shm_zone->data) = ctx;

   return NGX_OK;
}
#endif
