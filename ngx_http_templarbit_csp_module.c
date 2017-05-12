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

#include "templarbit/http.h"
#include "templarbit/json.h"
#include "templarbit/handler.h"

static char *ngx_http_templarbit_csp(ngx_conf_t *cf, void *cmd, void *conf);
static void * ngx_http_templarbit_csp_srv_conf(ngx_conf_t *cf);
static void * ngx_http_templarbit_csp_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_templarbit_csp_init(ngx_conf_t *cf);

static ngx_conf_post_handler_pt ngx_http_templarbit_csp_p = ngx_http_templarbit_csp;

static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;

static struct handler_node *templarbit_handlers;
pthread_mutex_t templarbit_handlers_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    ngx_str_t   token;
    ngx_str_t   property_id;
    ngx_int_t   fetch_interval;
    ngx_str_t   api_url;
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
    ngx_table_elt_t   *h;
    ngx_http_templarbit_csp_loc_conf_t *clcf;
    ngx_http_templarbit_csp_srv_conf_t *clsv;
    struct handler_node *handler;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_templarbit_csp_module);
    clsv = ngx_http_get_module_srv_conf(r, ngx_http_templarbit_csp_module);

    handler = handler_find_node(templarbit_handlers, (char*) clsv->token.data);

    if (!handler->csp) {
       handler->csp = shmat(handler->csp_shmid, (void *)0, 0);
    }

    if (clcf->enabled != 1) {
       return ngx_http_next_header_filter(r);
    }

    if (!handler->csp) {
       return ngx_http_next_header_filter(r);
    }

    if (r->headers_out.status != NGX_HTTP_OK) {
        return ngx_http_next_header_filter(r);
    }

    h = ngx_list_push(&r->headers_out.headers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    ngx_str_set(&h->key, "Content-Security-Policy");
    ngx_str_t csp = { (size_t) strlen(handler->csp), (unsigned char *) handler->csp };
    h->value = csp;
    h->hash = 1;

    return ngx_http_next_header_filter(r);
} 

void* poll_license_server(void *arg)
{
    long response_code = 0;
    char* token = arg;
    struct handler_node *handler;
    ngx_http_templarbit_csp_srv_conf_t *clsv;

    while (1)
    {
       handler = handler_find_node(templarbit_handlers, token);
       clsv = handler->clsv;

       // making request to templarbit server
       char* response_body = http_post(
          (char*) clsv->api_url.data,
          handler->request_body,
          &response_code,
          clsv->fetch_interval,
          clsv->fetch_interval);

       if (response_code != 200) {
          return NULL;
       }

       struct json_node *json = json_parse(response_body);
       if (!json) {
          return NULL;
       }

       struct json_node *csp = json_find_node(json, "csp");
       if (!csp) {
          return NULL;
       }

       char* new_csp = csp->value;

       if (!handler->csp // previous CSP is not set
           || (handler->csp && strcmp(handler->csp, csp->value))) // CSP are inequal
       {
          memset(handler->csp, 0, CSP_SIZE);
          strcpy(handler->csp, new_csp);
          //printf("CSP: %s (%p)\n", handler->csp, handler->csp);
       }

       json_free(csp);

       sleep(clsv->fetch_interval);
    }

    return NULL;
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
    pthread_t poller_thread;                  /* poller thread handler */
    struct handler_node *handler;

    // reading location and server configs
    clsv = ngx_http_conf_get_module_srv_conf(cf, ngx_http_templarbit_csp_module);

    // checking if we have already handler/poller for the specific token
    handler = handler_find_node(templarbit_handlers, (char*) clsv->token.data);

    if (handler) {
       return NGX_CONF_OK;
    }

    // preparing request body
    struct json_node *request_json = NULL;
    json_append_node_nv(&request_json, "token", (char*) clsv->token.data);
    json_append_node_nv(&request_json, "property_id", (char*) clsv->property_id.data);

    handler = handler_append_node_n(&templarbit_handlers, (char*) clsv->token.data);
    handler->request_body = json_serialize(request_json);
    handler->handler_status = 1;
    handler->clsv = clsv;

    // allocating shared memory segment for storing CSP header
    handler->csp_shmid = shmget(IPC_PRIVATE, CSP_SIZE, 0644 | IPC_CREAT);
    if (handler->csp_shmid == -1) {
        perror("Error initializing shared memory segment");
        exit(1);
    }

    handler->csp = shmat(handler->csp_shmid, (void *)0, 0);
    if (handler->csp == (char *)(-1)) {
        perror("Error attaching shared memory segment");
        exit(1);
    }

    // starting CSP poller thread
    pthread_create(&poller_thread, NULL, &poll_license_server, (char*) clsv->token.data);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_templarbit_csp_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_templarbit_csp_handler;

    return NGX_OK;
}
