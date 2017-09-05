/**
 * Generic linked list structure
 */
struct list_node
{
  struct list_node *next;
};

/**
 * Find node in a list
 * @root - top root node
 * @key  - search key
 * @cmpf - callback to compare, should return 0 if matches with the @key
 */
struct list_node* find_node(struct list_node* root, void* key, int (*cmpf)(struct list_node*, void*));

/**
 * Adds element to the end of existing linked list or creates new one if @root stores NULL
 * @root - pointer to top root node
 * @node - node to add
 */
void append_node(struct list_node** root, struct list_node* node);
