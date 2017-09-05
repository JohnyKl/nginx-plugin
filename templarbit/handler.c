#include <stdlib.h>
#include <string.h>
#include "handler.h"

static int handler_node_cmpf(struct list_node* raw_node, void* key) {
   return strcmp((char*) key, ((struct handler_node*) raw_node)->token);
}

void handler_append_node(struct handler_node** root, struct handler_node* node) {
   append_node((struct list_node**) root, (struct list_node*) node);
}

struct handler_node* handler_find_node(struct handler_node* root, char* token) {
   return (struct handler_node*) find_node((struct list_node*) root, token, &handler_node_cmpf);
}

struct handler_node* handler_append_node_n(struct handler_node** root, char* token)
{
   struct handler_node* node = calloc(1, sizeof(struct handler_node));
   node->token = strdup(token);
   node->request_body = NULL;
   node->next = NULL;

   handler_append_node(root, node);
   return node;
}
