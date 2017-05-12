#include <stdlib.h>
#include <string.h>
#include "handler.h"

struct handler_node* handler_find_node(struct handler_node* root, char* token)
{
   if (!root || !token) {
      return NULL;
   }

   struct handler_node *n = root;
   while (1)
   {
      if (!strcmp(token, root->token))
         return n;

      if (!n->next)
         return NULL;

      n = n->next;
   }
}

void handler_append_node(struct handler_node** root, struct handler_node* node)
{
   if (!node) {
      return;
   }

   if (!*root) {
      *root = node;
   }
   else
   {
      struct handler_node *n = *root;
      while (1)
      {
         if (!n->next)
            break;

          n = n->next;
      }

      n->next = node;
   }

   node->next = NULL;
}

struct handler_node* handler_append_node_n(struct handler_node** root, char* token)
{
   struct handler_node* node = calloc(1, sizeof(struct handler_node));
   node->token = strdup(token);
   node->handler_status = 0;
   node->request_body = NULL;
   node->next = NULL;

   handler_append_node(root, node);
   return node;
}
