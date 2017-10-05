#ifndef POLL_API_IMPL_H
#define POLL_API_IMPL_H

#include "handler.h"

#ifndef _TESTS
#define _NGX_SLAB_POOL_T(shm) ((ngx_slab_pool_t*)((ngx_shm_t*)shm)->addr)
#define _NGX_SHM_MUTEX(shm)  (&(_NGX_SLAB_POOL_T(shm)->mutex))
#endif

int poll_api_impl(struct handler_node *handler, char* url, char* request_body,
        int timeout);
int process_server_instance(void *cf, struct handler_node **handlers,
        char* token, char *property_id, void* server_config,
        void* ngx_http_templarbit_csp_module);

#endif
