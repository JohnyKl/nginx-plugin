/**
 * Generic linked list structure
 */
struct list_node
{
  struct list_node *next;
};

/**
 * Definition for list callback operations
 * Arguments:
 * 0 - list node
 * 1 - arbitrary optional pointer to pass to user callback function 
 */
typedef int (*LIST_CALLBACK)(struct list_node* node, void* uptr);

/**
 * Find node in a list
 * @root - top root node
 * @key  - search key
 * @callback - callback to compare, should return 0 if matches with the @key
 */
struct list_node* find_node(struct list_node* root, void* key, LIST_CALLBACK callback);

/**
 * Adds element to the end of existing linked list or creates new one if @root stores NULL
 * @root - pointer to top root node
 * @node - node to add
 */
void append_node(struct list_node** root, struct list_node* node);

/**
 * Allows to iterate over linked list and apply callback function to each element
 * @root  - pointer to top root node
 * @callback - callback to apply to each element
 * @uptr  - arbitrary pointer to pass to user callback function 
 */
void iterate_list(struct list_node* root, LIST_CALLBACK callback, void* uptr);
