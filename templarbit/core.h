#ifndef POLL_API_IMPL_H
#define POLL_API_IMPL_H

#include "handler.h"

#ifndef _TESTS
#define _NGX_SLAB_POOL_T(shm) ((ngx_slab_pool_t*)((ngx_shm_t*)shm)->addr)
#define _NGX_SHM_MUTEX(shm)  (&(_NGX_SLAB_POOL_T(shm)->mutex))
#endif

/**
 * Defines codes that poll_api_impl function may return
 */
enum
{
   REQ_NO_RESPONSE,
   REQ_FAILED,
   REQ_OK,
   REQ_INVALID_TOKENS,
   REQ_MALFORMED_RESPONSE,
   REQ_INVALID_RESPONSE
};

/**
 * Polls templarbit server for CSP headers
 * and swaps them in shared memory segment if they are updated
 *
 * @param handler instance of 'struct headers' representing unique instance of token poller
 * @param url URL to request CSP headers from
 * @param request_body request body sent under POST request to an URL specifid in url parametere
 * @param timeout connection/read timeout for the HTTP request
 * @param response_body pointer to store response body
 * @param response_code pointer to store response code
 * 
 * @return code from enum defined above
 */
int poll_api_impl(struct handler_node *handler, char* url, char* request_body, int timeout, 
         char** response_body, int* response_code);

/**
 * Creates instance of 'struct headers' including shared memory segment
 *
 * @param cf
 * @param handlers pointer to a root handler node
 * @param token templarbit pointer to value specified by templarbit_token configuration directive in nginx configuration file
 * @param property_id pointer to value specified by templarbit_property_id configuration directive in nginx configuration file
 * @param server_config pointer to ngx_http_templarbit_csp_srv_conf_t
 * @param ngx_http_templarbit_csp_module pointer to ngx_http_module_t
 *
 * @return
 */
int process_server_instance(void *cf, struct handler_node **handlers,
        char* token, char *property_id, void* server_config,
        void* ngx_http_templarbit_csp_module);

#endif
