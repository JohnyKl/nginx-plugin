#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <signal.h>

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>

#include "templarbit/core.h"
#include "templarbit/http.h"
#include "templarbit/handler.h"
#include "vendor/jansson/src/jansson.h"

ngx_conf_t *glob_cf = NULL;

static char *ngx_http_templarbit_csp(ngx_conf_t *cf, void *cmd, void *conf);
static void *ngx_http_templarbit_csp_srv_conf(ngx_conf_t *cf);
static void *ngx_http_templarbit_csp_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_templarbit_csp_init(ngx_conf_t *cf);
static void add_response_header(ngx_http_request_t *r, char* header_name, char* header_value);
static void* poll_api(void *arg);
static void run_poll_api_threads();

static ngx_int_t init_module(ngx_cycle_t *cycle);

static ngx_conf_post_handler_pt ngx_http_templarbit_csp_p =
      ngx_http_templarbit_csp;

static ngx_http_output_header_filter_pt ngx_http_next_header_filter;

static struct handler_node *templarbit_handlers;
pthread_mutex_t templarbit_handlers_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
   ngx_str_t token;
   ngx_str_t property_id;
   ngx_int_t fetch_interval;
   ngx_str_t api_url;
   struct headers headers;
} ngx_http_templarbit_csp_srv_conf_t;

typedef struct
{
   ngx_flag_t enabled;
} ngx_http_templarbit_csp_loc_conf_t;

/**
 * This module provided directive: templarbit_csp
 */
static ngx_command_t ngx_http_templarbit_csp_commands[] =
{
   {
         ngx_string("templarbit_token"), /* directive */
      NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1, /* server context and takes 1 argument*/
      ngx_conf_set_str_slot, /* argument is a string, validating it using standard nginx function */
      NGX_HTTP_SRV_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_srv_conf_t, token),
      NULL },
   {
         ngx_string("templarbit_property_id"), /* directive */
      NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1, /* server context and takes 1 argument*/
      ngx_conf_set_str_slot, /* argument is a string, validating it using standard nginx function */
      NGX_HTTP_SRV_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_srv_conf_t, property_id),
      NULL },
   {
         ngx_string("templarbit_fetch_interval"), /* directive */
      NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1, /* server context and takes 1 argument*/
      ngx_conf_set_num_slot, /* argument is an integer, validating it using standard nginx function */
      NGX_HTTP_SRV_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_srv_conf_t, fetch_interval),
      NULL },
   {
         ngx_string("templarbit_api_url"), /* directive */
      NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1, /* server context and takes 1 argument*/
      ngx_conf_set_str_slot, /* argument is a string, validating it using standard nginx function */
      NGX_HTTP_SRV_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_srv_conf_t, api_url),
      NULL },
   {
         ngx_string("templarbit_csp_enable"), /* directive */
      NGX_HTTP_LOC_CONF | NGX_CONF_FLAG, /* server context and takes 1 argument*/
      ngx_conf_set_flag_slot, /* argument is a bool, validating it using standard nginx function */
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_loc_conf_t, enabled),
      &ngx_http_templarbit_csp_p },

         ngx_null_command
      /* command termination */
      }
;

/* The module context. */
static ngx_http_module_t ngx_http_templarbit_csp_module_ctx =
{
   NULL, /* preconfiguration */
   ngx_http_templarbit_csp_init, /* postconfiguration */

   NULL, /* create main configuration */
   NULL, /* init main configuration */

   ngx_http_templarbit_csp_srv_conf, /* create server configuration */
   NULL, /* merge server configuration */

   ngx_http_templarbit_csp_loc_conf, /* create location configuration */
   NULL /* merge location configuration */
};

/* The function which initializes memory for the module configuration structure       
 */
static void *
ngx_http_templarbit_csp_srv_conf(ngx_conf_t *cf)
{
   ngx_http_templarbit_csp_srv_conf_t *conf;

   conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_templarbit_csp_srv_conf_t));
   if (conf == NULL)
   {
      return NULL;
   }

   conf->fetch_interval = NGX_CONF_UNSET;

   return conf;
}

static void *
ngx_http_templarbit_csp_loc_conf(ngx_conf_t *cf)
{
   ngx_http_templarbit_csp_loc_conf_t *conf;

   conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_templarbit_csp_loc_conf_t));
   if (conf == NULL)
   {
      return NULL;
   }

   conf->enabled = NGX_CONF_UNSET;

   return conf;
}

static ngx_int_t init_module(ngx_cycle_t *cycle)
{
   pthread_atfork(NULL, NULL, &run_poll_api_threads);
   return 0;
}

/* Module definition. */
ngx_module_t ngx_http_templarbit_csp_module =
{
   NGX_MODULE_V1,
   &ngx_http_templarbit_csp_module_ctx, /* module context */
   ngx_http_templarbit_csp_commands, /* module directives */
   NGX_HTTP_MODULE, /* module type */
   NULL, /* init master */
   init_module, /* init module */
   NULL, /* init process */
   NULL, /* init thread */
   NULL, /* exit thread */
   NULL, /* exit process */
   NULL, /* exit master */
   NGX_MODULE_V1_PADDING
};

/**
 * Custom templarbit header filter
 * Adds custom headers (Content-Security-Policy, Content-Security-Policy-Report-Only) based on API response
 *
 * @param r Pointer to the request structure. See http_request.h.
 * @return The status of the response generation.
 */
static ngx_int_t ngx_http_templarbit_csp_handler(ngx_http_request_t *r)
{
   ngx_http_templarbit_csp_loc_conf_t *clcf; /* location configuration */
   ngx_http_templarbit_csp_srv_conf_t *clsv; /* server configuration */

   struct handler_node *handler;

   clcf = ngx_http_get_module_loc_conf(r, ngx_http_templarbit_csp_module);
   clsv = ngx_http_get_module_srv_conf(r, ngx_http_templarbit_csp_module);

   // looking for appropriate handler node
   handler = handler_find_node(templarbit_handlers, (char*) clsv->token.data);

   if (!handler)
   {
      return ngx_http_next_header_filter(r);
   }

   // attaching shared memory segment upon first request processing
   if (!handler->csp_headers) {
      sleep(1);
   }

   //fetching headers if version changed
   int cached_version = clsv->headers.version, actual_version = handler->csp_headers->version;

   if (cached_version != actual_version)
   {
      ngx_shmtx_lock(_NGX_SHM_MUTEX(handler->shm));
      memcpy(&clsv->headers, handler->csp_headers, CSP_SIZE);
      ngx_shmtx_unlock(_NGX_SHM_MUTEX(handler->shm));
   }

   if (clcf->enabled != 1)
   {
      return ngx_http_next_header_filter(r);
   }

   if (!handler->csp_headers)
   {
      return ngx_http_next_header_filter(r);
   }

   if (r->headers_out.status != NGX_HTTP_OK)
   {
      return ngx_http_next_header_filter(r);
   }

   add_response_header(r, "Content-Security-Policy", clsv->headers.csp);
   add_response_header(r, "Content-Security-Policy-Report-Only",
         clsv->headers.csp_ro);

   return ngx_http_next_header_filter(r);
}

/**
 * Starts polling for all tokens/property_ids. This function should run only once, which is achieved
 * by using has_executed static variable
 */ 
static void run_poll_api_threads()
{
   static int has_executed = 0;

   if (has_executed)
   {
      return;
   }

   // Traversing all handlers and running threads for each
   struct handler_node *current = templarbit_handlers;
   while (current != NULL)
   {
      pthread_t poller_thread;
      int thread_status = pthread_create(&poller_thread, NULL, poll_api, (void*) current->token);

      if (thread_status != 0)
      {
         ngx_log_stderr(0, "Was unable to create thread for handler %s/%s", current->token);
      }

      current = current->next;
   }

   has_executed = 1;
}

/**
 * Adds response header to a response for a given nginx request represented by pointer to ngx_http_request_t
 *
 * @param r nginx request for which response header should be injected
 * @param header_name name of header to inject
 * @param header_value value of header to inject
 */
static void add_response_header(ngx_http_request_t *r, char* header_name, char* header_value)
{
   ngx_table_elt_t *h;

   if (!header_name || !header_value)
   {
      return;
   }

   if (!strlen(header_name) || !strlen(header_value))
   {
      return;
   }

   h = ngx_list_push(&r->headers_out.headers);
   if (h == NULL)
   {
      return;
   }

   ngx_str_t key =
   {
      (size_t) strlen(header_name),
      (unsigned char *) header_name
   };

   ngx_str_t value =
   {
      (size_t) strlen(header_value),
      (unsigned char *) header_value
   };

   h->key = key;
   h->value = value;
   h->hash = 1;
}

/**
 * Starts templarbit API polling process per specific token/property_id
 */
static void* poll_api(void *arg)
{
   char* token = arg;
   struct handler_node *handler = handler_find_node(templarbit_handlers, token);

   if (handler == NULL)
   {
      ngx_log_stderr(0, "Templarbit: poll_api terminated as handler is not set");
      return NULL;
   }

   ngx_http_templarbit_csp_srv_conf_t *clsv = handler->clsv;

   ngx_log_stderr(0,
         "Templarbit: Successfully started API poller thread for token=%s, property_id=%s",
         clsv->token.data, clsv->property_id.data);

   while (1)
   {
      char* response_body = NULL;
      int response_code;

      int result = poll_api_impl(handler, (char*) clsv->api_url.data,
            handler->request_body, clsv->fetch_interval, &response_body, &response_code);

      switch (result)
      {
         case REQ_NO_RESPONSE:
            ngx_log_stderr(0,
                  "Templarbit: No response received from API server. Source request: '%s', destination URL: '%s'",
                  handler->request_body, (char*) clsv->api_url.data);
         case REQ_FAILED:
            ngx_log_stderr(0,
                  "Templarbit: Error received from API server. Response code: %d, response body: '%s', source request: '%s', destination URL: '%s'",
                  response_code, (response_body) ? response_body : "", handler->request_body,
                  (char*) clsv->api_url.data);
         case REQ_MALFORMED_RESPONSE:
            ngx_log_stderr(0,
                  "Templarbit: Invalid JSON received from API server. Source request: '%s', response body: '%s'.",
                  handler->request_body, (response_body) ? response_body : "");
         case REQ_INVALID_RESPONSE:
            ngx_log_stderr(0,
                  "Templarbit: Received JSON from API server doesn't have neither "
                        "Content-Security-Policy nor Content-Security-Policy-Report-Only set. "
                        "Source request: '%s', response body: '%s'",
                  handler->request_body, (response_body) ? response_body : "");
      }

      free(response_body);
      sleep(clsv->fetch_interval);
   }

   ngx_log_stderr(0, "Templarbit: API poller thread exited!");
   return NULL;
}

/**
 * Configuration setup function that installs the content handler.
 *
 * @param cf Module configuration structure pointer.
 * @param cmd Module directives structure pointer.
 * @param conf Module configuration structure pointer.
 * @return Status of the configuration setup.
 */
char *ngx_http_templarbit_csp(ngx_conf_t *cf, void *cmd, void *conf)
{
   ngx_http_templarbit_csp_srv_conf_t *clsv; /* pointer to core server configuration */
   char *token = NULL;
   char *property_id = NULL;
   glob_cf = cf;

   ngx_log_stderr(0, "Templarbit: Starting server instance configuration");

   // reading location and server configs
   clsv = ngx_http_conf_get_module_srv_conf(cf, ngx_http_templarbit_csp_module);

   token = (char*) clsv->token.data;
   property_id = (char*) clsv->property_id.data;

   ngx_log_stderr(0,
         "Templarbit: Server instance has token=%s, property_id=%s configured",
         token, property_id);

   int status = process_server_instance(cf, &templarbit_handlers, token,
         property_id, clsv, &ngx_http_templarbit_csp_module);

   switch (status)
   {
      case HANDLER_EXISTS:
         return NGX_CONF_OK;
      case HANDLER_SHM_INIT_FAILED:
         ngx_log_error(NGX_LOG_EMERG, cf->log, 0,
               "Templarbit: Was unable to allocate shared memory segment of %ld size",
               CSP_SIZE);
         exit(1);
      case HANDLER_SHM_ATTACH_FAILED:
         ngx_log_error(NGX_LOG_EMERG, cf->log, 0,
               "Templarbit: Was unable to attach shared memory segment");
         exit(1);
      case HANDLER_THREAD_FAILED:
         ngx_log_error(NGX_LOG_EMERG, cf->log, 0,
               "Templarbit: Was unable to start API poller thread for token=%s, property_id=%s",
               token, property_id);
         exit(1);
      case HANDLER_STARTED:
         ngx_log_error(NGX_LOG_NOTICE, cf->log, 0,
               "Templarbit: Server instance configuration finished");
         break;
   }

   return NGX_CONF_OK;
}

/**
 * Sets custom templarbit header filter 
 */ 
static ngx_int_t ngx_http_templarbit_csp_init(ngx_conf_t *cf)
{
   ngx_http_next_header_filter = ngx_http_top_header_filter;
   ngx_http_top_header_filter = ngx_http_templarbit_csp_handler;

   return NGX_OK;
}
