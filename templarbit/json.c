#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../jsmn/jsmn.h"
#include "json.h"

struct json_node* json_parse_token(char* raw_json, jsmntok_t* key_token, jsmntok_t* value_token)
{
   struct json_node* node = calloc(1, sizeof(struct json_node));

   int key_length = key_token->end - key_token->start,
       value_length = value_token->end - value_token->start;

   char* key = calloc(key_length + 1, sizeof(char));
   char* value = calloc(value_length + 1, sizeof(char));

   strncpy(key, raw_json + key_token->start, key_length);
   strncpy(value, raw_json + value_token->start, value_length);

   node->name = key;
   node->value = value;
   node->next = NULL;

   return node;
}

struct json_node* json_find_node(struct json_node* root, char* name)
{
   if (!root || !name) {
      return NULL;
   }

   struct json_node *n = root;
   while (1)
   {
      if (!strcmp(name, n->name))
         return n;

      if (!n->next)
         return NULL;

      n = n->next;
   }
}

char* json_serialize(struct json_node* root)
{
   if (!root)
      return NULL;

   int isize = 3, cpos = 0;
   char* result = malloc(isize * sizeof(char));

   cpos += sprintf(result, "{");

   struct json_node *n = root;
   while (1)
   {
      int tlen = strlen(n->name)
                 + strlen(n->value) 
                 + (n->next ? 1 : 0) //comma
                 + 5; //quotes and colon

      isize += tlen;
      result = realloc(result, isize);

      cpos += sprintf(result+cpos, "\"%s\":\"%s\"", n->name, n->value);
      if (n->next) {
         cpos += sprintf(result+cpos, ",");
         n = n->next;
      }
      else
         break;
   }
  
   cpos += sprintf(result+cpos, "}"); 
   result[cpos] = (char)0;

   return result;
}

void json_free(struct json_node* root)
{
   if (!root)
      return;

   struct json_node *n = root;
   do
   {
      struct json_node *p = n->next;
      free(n->name);
      free(n->value);
      free(n);

      n = p;
   }
   while (n);
}

void json_append_node_nv(struct json_node** root, char* name, char* value)
{
   struct json_node* node = calloc(1, sizeof(struct json_node));
   node->name = strdup(name);
   node->value = strdup(value);
   node->next = NULL;

   json_append_node(root, node);
}

void json_append_node(struct json_node** root, struct json_node* node)
{
   if (!node) {
      return;
   }

   if (!*root) {
      *root = node;
   }
   else
   {
      struct json_node *n = *root;
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

struct json_node* json_parse(char* raw_json)
{
   int i, r;

   jsmn_parser p;
   jsmntok_t t[8]; /* We expect no more than 8 tokens */

   jsmn_init(&p);
   r = jsmn_parse(&p, raw_json, strlen(raw_json), t, sizeof(t)/sizeof(t[0]));

   if (r < 0) {
      return NULL;
   }

   /* Assume the top-level element is an object */
   if (r < 1 || t[0].type != JSMN_OBJECT) {
      return NULL;
   }

   struct json_node* root = NULL;

   /* Loop over all keys of the root object */
   for (i = 1; i < r; i++)
   {
      if (t[i].type != JSMN_STRING) 
         continue;

      struct json_node* node = json_parse_token(raw_json, &t[i], &t[i+1]);
      json_append_node(&root, node);

      i++;
   }

   return root;
}
