#ifndef _HANDLER_H_
#define _HANDLER_H_ 

#include <sys/types.h>
#include <semaphore.h>
#include "list.h"

/**
 * Offset from the begigging of "struct headers"
 * to the first header declaration
 */
#define CSP_OFFSET sizeof(int)

/**
 * Max injected header size
 */
#define CSP_HEADER_SIZE 8192

/**
 * Size of all structure that contains all injected headers
 */
#define CSP_SIZE CSP_HEADER_SIZE*2+CSP_OFFSET

/**
 * Statuses that could occur during 
 * handler initialization phase (after nginx reads configuration)
 */
enum {
   /**
    * Handler has already been created for the specific token
    */
   HANDLER_EXISTS,

   /**
    * Handler for the specific token haven't been created yet
    */
   HANDLER_DOES_NOT_EXIST,

   /**
    * Was unable to init shared memory segment
    */
   HANDLER_SHM_INIT_FAILED,

   /**
    * Shared memory segment attach failed 
    */
   HANDLER_SHM_ATTACH_FAILED,

   /**
    * Shared memory lock acquisition failed
    */
   HANDLER_LCK_FAILED,

   /**
    * Processing handler thread failed to start
    */
   HANDLER_THREAD_FAILED,

   /**
    * Handler has successfully started
    */
   HANDLER_STARTED,
};

/**
 * Container for headers injected in responses
 * for eligible server instances
 */
struct headers
{
  /**
   * Version of headers snapshot
   * Incremented on headers change
   */
  int version;

  /**
   * Stores Content-Security-Policy header value
   */
  char csp[CSP_HEADER_SIZE];

  /**
   * Stores Content-Security-Policy-Report-Only header value
   */
  char csp_ro[CSP_HEADER_SIZE];
};

struct handler_node
{
  /**
   * Next item
   */
  struct handler_node* next;

  char* token;

  struct headers *csp_headers;
  
  char* request_body;

  void *clsv;

  void* shm_zone;
  void* shm_pool;
  void* shm;
};

struct handler_node* handler_find_node(struct handler_node* root, char* token);
void handler_append_node(struct handler_node** root, struct handler_node* node);
struct handler_node* handler_append_node_n(struct handler_node** root, char* token);

#endif
