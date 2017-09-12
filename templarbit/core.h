#ifndef POLL_API_IMPL_H
#define POLL_API_IMPL_H

#include "handler.h"

int poll_api_impl(struct handler_node *handler, char* url, char* request_body, int timeout);
int process_server_instance(struct handler_node **handlers, char* token, char *property_id, void* server_config, void*(*thread_function)(void*));

#endif
