#include <stdlib.h>
#include "list.h"

struct list_node* find_node(struct list_node* root, void* key, LIST_CALLBACK callback)
{
   if (!root || !callback || !key) {
      return NULL;
   }

   struct list_node *n = root;
   while (1)
   {
      if (!callback(n, key))
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

void iterate_list(struct list_node* root, LIST_CALLBACK callback, void* uptr)
{
   if (!root || !callback) {
      return;
   }

   struct list_node *current = root, *next = NULL;
   while (1)
   {
      next = current->next;
      callback(current, uptr);

      if (!next) {
         return;
      }

      current = next;
   }
}

