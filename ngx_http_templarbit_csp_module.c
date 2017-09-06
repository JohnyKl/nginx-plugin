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

#include "templarbit/request.h"
#include "templarbit/http.h"
#include "templarbit/handler.h"
#include "vendor/jansson/src/jansson.h"


static char *ngx_http_templarbit_csp(ngx_conf_t *cf, void *cmd, void *conf);
static void * ngx_http_templarbit_csp_srv_conf(ngx_conf_t *cf);
static void * ngx_http_templarbit_csp_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_templarbit_csp_init(ngx_conf_t *cf);
static void add_response_header(ngx_http_request_t *r, char* header_name, char* header_value);
static int process_server_instance(struct handler_node **handlers, char* token, char *property_id, void* server_config);
static void* poll_api(void *arg);
static int poll_api_impl(struct handler_node *handler, char* url, char* request_body, int timeout);

static ngx_conf_post_handler_pt ngx_http_templarbit_csp_p = ngx_http_templarbit_csp;

static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;

static struct handler_node *templarbit_handlers;
pthread_mutex_t templarbit_handlers_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    ngx_str_t      token;
    ngx_str_t      property_id;
    ngx_int_t      fetch_interval;
    ngx_str_t      api_url;
    struct headers headers;
} ngx_http_templarbit_csp_srv_conf_t;

typedef struct {
    ngx_flag_t   enabled;
} ngx_http_templarbit_csp_loc_conf_t;

/**
 * This module provided directive: templarbit_csp
 */
static ngx_command_t ngx_http_templarbit_csp_commands[] =
{
    {
      ngx_string("templarbit_token"), /* directive */
      NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1, /* server context and takes 1 argument*/
      ngx_conf_set_str_slot, /* argument is a string, validating it using standard nginx function */
      NGX_HTTP_SRV_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_srv_conf_t, token), 
      NULL
    },
    {
      ngx_string("templarbit_property_id"), /* directive */
      NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1, /* server context and takes 1 argument*/
      ngx_conf_set_str_slot, /* argument is a string, validating it using standard nginx function */
      NGX_HTTP_SRV_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_srv_conf_t, property_id),
      NULL
    },
    {
      ngx_string("templarbit_fetch_interval"), /* directive */
      NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1, /* server context and takes 1 argument*/
      ngx_conf_set_num_slot, /* argument is an integer, validating it using standard nginx function */
      NGX_HTTP_SRV_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_srv_conf_t, fetch_interval),
      NULL
    },
    {
      ngx_string("templarbit_api_url"), /* directive */
      NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1, /* server context and takes 1 argument*/
      ngx_conf_set_str_slot, /* argument is a string, validating it using standard nginx function */
      NGX_HTTP_SRV_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_srv_conf_t, api_url),
      NULL
    },
    {
      ngx_string("templarbit_csp_enable"), /* directive */
      NGX_HTTP_LOC_CONF|NGX_CONF_FLAG, /* server context and takes 1 argument*/
      ngx_conf_set_flag_slot, /* argument is a bool, validating it using standard nginx function */
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_templarbit_csp_loc_conf_t, enabled),
      &ngx_http_templarbit_csp_p
    },

    ngx_null_command /* command termination */
};

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
    NULL  /* merge location configuration */
};

/* The function which initializes memory for the module configuration structure       
 */
static void *
ngx_http_templarbit_csp_srv_conf(ngx_conf_t *cf)
{
    ngx_http_templarbit_csp_srv_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_templarbit_csp_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->fetch_interval = NGX_CONF_UNSET;
    
    return conf;
}

static void *
ngx_http_templarbit_csp_loc_conf(ngx_conf_t *cf)
{
    ngx_http_templarbit_csp_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_templarbit_csp_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enabled = NGX_CONF_UNSET;
    
    return conf;
}

/* Module definition. */
ngx_module_t ngx_http_templarbit_csp_module =
{
    NGX_MODULE_V1,
    &ngx_http_templarbit_csp_module_ctx, /* module context */
    ngx_http_templarbit_csp_commands, /* module directives */
    NGX_HTTP_MODULE, /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};

/**
 * Content handler.
 *
 * @param r
 *   Pointer to the request structure. See http_request.h.
 * @return
 *   The status of the response generation.
 */
static ngx_int_t
ngx_http_templarbit_csp_handler(ngx_http_request_t *r)
{
    ngx_http_templarbit_csp_loc_conf_t *clcf;
    ngx_http_templarbit_csp_srv_conf_t *clsv;
    struct handler_node *handler;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_templarbit_csp_module);
    clsv = ngx_http_get_module_srv_conf(r, ngx_http_templarbit_csp_module);

    // looking for appropriate handler node
    handler = handler_find_node(templarbit_handlers, (char*) clsv->token.data);

    // attaching shared memory segment upin first request processing
    if (!handler->csp_headers) {
       handler->csp_headers = shmat(handler->csp_shmid, (void *)0, 0);
    }

    // fetching headers if version changed
    int cached_version = clsv->headers.version,
        actual_version = handler->csp_headers->version;

    if (cached_version != actual_version) {
       sem_wait(handler->csp_semid);
       memcpy(&clsv->headers, handler->csp_headers, CSP_SIZE);
       sem_post(handler->csp_semid);
    }

    if (clcf->enabled != 1) {
       return ngx_http_next_header_filter(r);
    }

    if (!handler->csp_headers) {
       return ngx_http_next_header_filter(r);
    }

    if (r->headers_out.status != NGX_HTTP_OK) {
        return ngx_http_next_header_filter(r);
    }

    add_response_header(r, "Content-Security-Policy", clsv->headers.csp);
    add_response_header(r, "Content-Security-Policy-Report-Only", clsv->headers.csp_ro);

    return ngx_http_next_header_filter(r);
} 

static void
add_response_header(ngx_http_request_t *r, char* header_name, char* header_value)
{
    ngx_table_elt_t *h;

    if (!header_name || !header_value) {
        return;
    }

    if (!strlen(header_name) || !strlen(header_value)) {
        return;
    }

    h = ngx_list_push(&r->headers_out.headers);
    if (h == NULL) {
        return;
    }

    ngx_str_t key = { (size_t) strlen(header_name), (unsigned char *) header_name }; 
    ngx_str_t value = { (size_t) strlen(header_value), (unsigned char *) header_value };

    h->key = key;
    h->value = value;
    h->hash = 1;
}

static void*
poll_api(void *arg)
{
    char* token = arg;
    struct handler_node *handler = handler_find_node(templarbit_handlers, token);

    ngx_http_templarbit_csp_srv_conf_t *clsv = handler->clsv;

    ngx_log_stderr(0, "Templarbit: Successfully started API poller thread for token=%s, property_id=%s",
            clsv->token.data, clsv->property_id.data);

    while (1)
    {
       int result = poll_api_impl(
            handler,
            (char*) clsv->api_url.data,
            handler->request_body,
            clsv->fetch_interval);

       switch (result)
       {
          case REQ_NO_RESPONSE:
             ngx_log_stderr(0, "Templarbit: No response received from API server. Source request: '%s', destination URL: '%s'",
                 handler->request_body, (char*) clsv->api_url.data);
          case REQ_FAILED:
             ngx_log_stderr(0, "Templarbit: Error received from API server. Response code: %d, response body: '%s', source request: '%s', destination URL: '%s'",
                 /*response_code, response_body*/0,"", handler->request_body, (char*) clsv->api_url.data);
          case REQ_MALFORMED_RESPONSE:
             ngx_log_stderr(0, "Templarbit: Invalid JSON received from API server. Source request: '%s', response body: '%s'. Error on line %d: %s",
                 handler->request_body, /*response_body*/"", /*json_error.line, json_error.text*/0,"");
          case REQ_INVALID_RESPONSE:
             ngx_log_stderr(0, "Templarbit: Received JSON from API server doesn't have neither "
                 "Content-Security-Policy nor Content-Security-Policy-Report-Only set. "
                 "Source request: '%s', response body: '%s'", handler->request_body, /*response_body*/"");
       }

       sleep(clsv->fetch_interval);
    }

    return NULL;
}

static
int poll_api_impl(struct handler_node *handler, char* url, char* request_body, int timeout)
{
    json_error_t json_error;
    json_t *root_node = NULL;
    int result = REQ_OK;

    // constructing request struct
    http_header_t* headers = NULL;
    header_append_node_n(&headers, "Content-Type: application/json");
    http_request_t* request = make_http_request(url, request_body, POST, headers, timeout, timeout);

    // making request to templarbit server
    http_response_t* response = http_post(request);

    if (!response || !response->response_body) {
       result = REQ_NO_RESPONSE;
       goto finalize_request;
    }

    if (response->response_code != 200) {
       result = REQ_FAILED;
       goto finalize_request;
    }

    root_node = json_loads(response->response_body, 0, &json_error);
    if (!root_node) {
       result = REQ_MALFORMED_RESPONSE;
       goto finalize_request;
    }

    json_t *raw_csp = json_object_get(root_node, "csp"),
           *raw_csp_ro = json_object_get(root_node, "csp_report_only");

    if (!json_is_string(raw_csp) && !json_is_string(raw_csp_ro)) {
       result = REQ_INVALID_RESPONSE;
       goto finalize_request;
    }

    const char *new_csp = json_string_value(raw_csp),
               *new_csp_ro = json_string_value(raw_csp_ro);

    char *curr_csp = handler->csp_headers->csp,
         *curr_csp_ro = handler->csp_headers->csp_ro;

    if (strcmp(curr_csp, new_csp) // CSP are inequal
        || strcmp(curr_csp_ro, new_csp_ro)) // CSP RO are inequal
    {
       // incrementing version number
       handler->csp_headers->version = handler->csp_headers->version + 1;

       // locking shared memory segment for update operation
       sem_wait(handler->csp_semid);

       if (new_csp) {
          memset(curr_csp, 0, CSP_HEADER_SIZE);
          strcpy(curr_csp, new_csp);
       }

       if (new_csp_ro) {
          memset(curr_csp_ro, 0, CSP_HEADER_SIZE);
          strcpy(curr_csp_ro, new_csp_ro);
       }

       // releasing shared memory segment lock
       sem_post(handler->csp_semid);

       ngx_log_stderr(0, "Templarbit: New headers received. Content-Security-Policy: '%s', Content-Security-Policy-Report-Only: '%s'",
          new_csp, new_csp_ro);
    }

    finalize_request:
    json_decref(root_node);
    free_http_request(request);
    free_http_response(response);

    return result;
}

/**
 * Configuration setup function that installs the content handler.
 *
 * @param cf
 *   Module configuration structure pointer.
 * @param cmd
 *   Module directives structure pointer.
 * @param conf
 *   Module configuration structure pointer.
 * @return string
 *   Status of the configuration setup.
 */
char *ngx_http_templarbit_csp(ngx_conf_t *cf, void *cmd, void *conf)
{
    ngx_http_templarbit_csp_srv_conf_t *clsv; /* pointer to core server configuration */
    char *token = NULL;
    char *property_id = NULL;

    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "Templarbit: Starting server instance configuration");

    // reading location and server configs
    clsv = ngx_http_conf_get_module_srv_conf(cf, ngx_http_templarbit_csp_module);

    token = (char*) clsv->token.data;
    property_id = (char*) clsv->property_id.data;

    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "Templarbit: Server instance has token=%s, property_id=%s configured", token, property_id);

    int status = process_server_instance(
       &templarbit_handlers,
       token,
       property_id,
       clsv);

    switch (status)
    {
       case HANDLER_EXISTS:
          return NGX_CONF_OK;
       case HANDLER_SHM_INIT_FAILED:
          ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "Templarbit: Was unable to allocate shared memory segment of %ld size", CSP_SIZE);
          exit(1);
       case HANDLER_SHM_ATTACH_FAILED:
          ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "Templarbit: Was unable to attach shared memory segment");
          exit(1);
       case HANDLER_THREAD_FAILED:
          ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "Templarbit: Was unable to start API poller thread for token=%s, property_id=%s", 
                     token, property_id);
          exit(1);
       case HANDLER_STARTED:
          ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "Templarbit: Server instance configuration finished");
          break;
    }

    return NGX_CONF_OK;
}

static
int process_server_instance(struct handler_node **handlers, char* token, char *property_id, void* server_config)
{
    pthread_t poller_thread;                  /* poller thread handler */
    int thread_status;                        /* poller thread creation status */
    struct handler_node *handler;             /* templarbit server handler node */

    // checking if we have already handler/poller for the specific token
    handler = handler_find_node(*handlers, token);
    if (handler) {
       return HANDLER_EXISTS;
    }

    // preparing request body
    json_t *request = json_pack("{ssss}", "token", token, "property_id", property_id);

    handler = handler_append_node_n(handlers, token);
    handler->request_body = json_dumps(request, JSON_COMPACT);
    handler->clsv = server_config;

    // allocating shared memory segment for storing CSP header
    handler->csp_shmid = shmget(IPC_PRIVATE, CSP_SIZE, 0644 | IPC_CREAT);
    if (handler->csp_shmid == -1) {
        return HANDLER_SHM_INIT_FAILED;
    }

    handler->csp_headers = shmat(handler->csp_shmid, (void *)0, 0);
    if ((char*) handler->csp_headers == (char *)(-1)) {
        return HANDLER_SHM_ATTACH_FAILED;
    }

    // creating semaphore in order to synchronize access to shared memory segment
    handler->csp_semid = sem_open(token, O_CREAT | O_EXCL, 0644, 1);

    // unlink prevents the semaphore existing forever
    sem_unlink(token); 

    // starting CSP poller thread
    thread_status = pthread_create(&poller_thread, NULL, &poll_api, token);
    if (thread_status != 0) {
        return HANDLER_THREAD_FAILED;
    }

    return HANDLER_STARTED;
}

static ngx_int_t
ngx_http_templarbit_csp_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_templarbit_csp_handler;

    return NGX_OK;
}
