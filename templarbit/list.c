#include <stdlib.h>
#include "list.h"

struct list_node* find_node(struct list_node* root, void* key, int (*cmpf)(struct list_node*, void*))
{
   if (!root || !cmpf || !key) {
      return NULL;
   }

   struct list_node *n = root;
   while (1)
   {
      if (!cmpf(n, key))
         return n;

      if (!n->next)
         return NULL;

      n = n->next;
   }
}

void append_node(struct list_node** root, struct list_node* node)
{
   if (!node) {
      return;
   }

   if (!*root) {
      *root = node;
   }
   else
   {
      struct list_node *n = *root;
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

