
#define CSP_SIZE 10240

struct handler_node
{
  char* token;

  int csp_shmid;
  char *csp;
  
  char* request_body;
  int handler_status;
  void *clsv;
  struct handler_node* next;
};

struct handler_node* handler_find_node(struct handler_node* root, char* token);
void handler_append_node(struct handler_node** root, struct handler_node* node);
struct handler_node* handler_append_node_n(struct handler_node** root, char* token);
