#include <sys/types.h>
#include <semaphore.h>

#define CSP_OFFSET sizeof(int)
#define CSP_HEADER_SIZE 8192
#define CSP_SIZE CSP_HEADER_SIZE*2+CSP_OFFSET

struct headers
{
  int version;
  char csp[CSP_HEADER_SIZE];
  char csp_ro[CSP_HEADER_SIZE];
};

struct handler_node
{
  char* token;

  int csp_shmid;
  sem_t *csp_semid;
  struct headers *csp_headers;
  
  char* request_body;
  int handler_status;

  void *clsv;
  void *clng;

  struct handler_node* next;
};

struct handler_node* handler_find_node(struct handler_node* root, char* token);
void handler_append_node(struct handler_node** root, struct handler_node* node);
struct handler_node* handler_append_node_n(struct handler_node** root, char* token);
