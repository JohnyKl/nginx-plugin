
struct json_node
{
  char* name;
  char* value;
  struct json_node* next;
};

struct json_node* json_parse(char* raw_json);
char* json_serialize(struct json_node* root);
struct json_node* json_find_node(struct json_node* root, char* name);
void json_append_node(struct json_node** root, struct json_node* node);
void json_append_node_nv(struct json_node** root, char* name, char* value);
void json_free(struct json_node* root);
