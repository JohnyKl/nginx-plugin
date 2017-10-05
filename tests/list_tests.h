#ifndef LIST_TESTS_H_
#define LIST_TESTS_H_

#include <string.h>

#include "../templarbit/list.h"

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

#include <curl/curl.h>

struct mock_list_node
{
   struct mock_list_node *next;
   int data;
};

int list_find_mock_callback(struct list_node* node, void* uptr);
int list_iterate_mock_callback(struct list_node* node, void* uptr);
int list_iterate_not_called_mock_callback(struct list_node* node, void* uptr);
int list_iterate_null_parameter_mock_callback(struct list_node* node,
      void* uptr);

void create_list(struct mock_list_node** root, struct mock_list_node* nodesArr,
      int size);
/************* Test case functions prototypes ****************/

void test_list_append_null_node_head(void);
void test_list_append_null_node(void);
void test_list_append_node_head(void);
void test_list_append_node_second(void);
void test_list_append_node_tail(void);
void test_list_find_exist_node(void);
void test_list_find_not_exist_node(void);
void test_list_find_null_parameters(void);
void test_list_iterate(void);
void test_list_iterate_null_parameters(void);

/************* Test case functions ****************/

void test_list_append_null_node_head(void)
{
   struct list_node* root = NULL;

   append_node(&root, NULL);

   CU_ASSERT_PTR_NULL(root);
}

void test_list_append_null_node(void)
{
   struct list_node* root = NULL;
   struct list_node newNode;
   newNode.next = &newNode; // check if we do nothing with list while append NULL node pointer
   root = &newNode;

   append_node(&root, NULL);

   CU_ASSERT_PTR_NOT_NULL_FATAL(root);
   CU_ASSERT_PTR_EQUAL(root, &newNode);
   CU_ASSERT_PTR_EQUAL(root->next, &newNode);
   CU_ASSERT_PTR_EQUAL(newNode.next, &newNode);
}

void test_list_append_node_head(void)
{
   struct list_node* root = NULL;
   struct list_node newNode;
   newNode.next = &newNode; // check if pointer to the next node became NULL for the tail node

   append_node(&root, &newNode);

   CU_ASSERT_PTR_NOT_NULL_FATAL(root);
   CU_ASSERT_PTR_EQUAL(root, &newNode);
   CU_ASSERT_PTR_NULL(root->next);
   CU_ASSERT_PTR_NULL(newNode.next);
}

void test_list_append_node_second(void)
{
   struct list_node* root = NULL;
   struct list_node newNode1, newNode2;
   memset(&newNode1, 0, sizeof(struct list_node));
   memset(&newNode2, 0, sizeof(struct list_node));

   root = &newNode1;
   newNode1.next = NULL;

   append_node(&root, &newNode2);

   CU_ASSERT_PTR_NOT_NULL_FATAL(root);
   CU_ASSERT_PTR_EQUAL(root, &newNode1);
   CU_ASSERT_PTR_EQUAL(root->next, &newNode2);
   CU_ASSERT_PTR_EQUAL(newNode1.next, &newNode2);
   CU_ASSERT_PTR_NULL(root->next->next);
   CU_ASSERT_PTR_NULL(newNode2.next);
}

void test_list_append_node_tail(void)
{
   struct list_node* root = NULL;
   struct list_node newNode1, newNode2, newNode3;
   memset(&newNode1, 0, sizeof(struct list_node));
   memset(&newNode2, 0, sizeof(struct list_node));
   memset(&newNode3, 0, sizeof(struct list_node));

   root = &newNode1;
   newNode1.next = &newNode2;

   append_node(&root, &newNode3);

   CU_ASSERT_PTR_NOT_NULL_FATAL(root);
   CU_ASSERT_PTR_EQUAL(root, &newNode1);

   CU_ASSERT_PTR_EQUAL(root->next, &newNode2);
   CU_ASSERT_PTR_EQUAL(newNode1.next, &newNode2);

   CU_ASSERT_PTR_EQUAL(root->next->next, &newNode3);
   CU_ASSERT_PTR_EQUAL(newNode2.next, &newNode3);

   CU_ASSERT_PTR_NULL(root->next->next->next);
   CU_ASSERT_PTR_NULL(newNode3.next);
}

#ifndef TEST_NODES_ARRAY_NODES_COUNT
#define TEST_NODES_ARRAY_NODES_COUNT 6
#endif

void test_list_find_exist_node(void)
{
   struct mock_list_node nodes[TEST_NODES_ARRAY_NODES_COUNT];
   struct mock_list_node* root = NULL;
   create_list(&root, nodes, TEST_NODES_ARRAY_NODES_COUNT);
   int existedKey = 5;

   struct list_node* foundNode = find_node((struct list_node*) root,
         &existedKey, list_find_mock_callback);

   CU_ASSERT_PTR_NOT_NULL(foundNode);
}

void test_list_find_not_exist_node(void)
{
   struct mock_list_node nodes[TEST_NODES_ARRAY_NODES_COUNT];
   struct mock_list_node* root = NULL;
   create_list(&root, nodes, TEST_NODES_ARRAY_NODES_COUNT);
   int notExistedKey = TEST_NODES_ARRAY_NODES_COUNT + 1;

   struct list_node* foundNode = find_node((struct list_node*) root,
         &notExistedKey, list_find_mock_callback);

   CU_ASSERT_PTR_NULL(foundNode);
}

void test_list_find_null_parameters(void)
{
   struct list_node node;
   struct list_node* root = &node;

   struct list_node* foundNodeNullRoot = find_node(NULL, "not_existed_key",
         (LIST_CALLBACK) list_find_mock_callback);
   struct list_node* foundNodeNUllKey = find_node(root, NULL,
         list_find_mock_callback);
   struct list_node* foundNodeNUllCallback = find_node(root, "not_existed_key",
         (LIST_CALLBACK) NULL);

   CU_ASSERT_PTR_NULL(foundNodeNullRoot);
   CU_ASSERT_PTR_NULL(foundNodeNUllKey);
   CU_ASSERT_PTR_NULL(foundNodeNUllCallback);
}

void test_list_iterate(void)
{
   int callbackParam = 0;
   struct mock_list_node nodes[TEST_NODES_ARRAY_NODES_COUNT];
   struct mock_list_node* root = NULL;
   create_list(&root, nodes, TEST_NODES_ARRAY_NODES_COUNT);

   iterate_list((struct list_node*) root, list_iterate_mock_callback,
         &callbackParam);

   struct mock_list_node* curr = root;
   for (int i = 0; i < TEST_NODES_ARRAY_NODES_COUNT && curr != NULL; i++)
   {
      CU_ASSERT_EQUAL(curr->data, callbackParam);
      curr = curr->next;
   }
}

void test_list_iterate_null_parameters(void)
{
   int callbackParam = 90;
   int nodeValue = 5555;
   struct mock_list_node node;
   struct list_node* root = (list_node*) &node;
   node.data = nodeValue;
   node.next = NULL;

   /* Check that callback function must not be called if root == NULL*/
   iterate_list(NULL, list_iterate_not_called_mock_callback, &callbackParam); //callback function call CU_FAIL if it is called
   iterate_list(root, list_iterate_null_parameter_mock_callback, NULL); //callback function call CU_FAIL if parameter pointer is not NULL

   iterate_list(root, NULL, &callbackParam);
   /* check that nothing happen when callback function is NULL*/
   CU_ASSERT_PTR_NOT_NULL_FATAL(root);
   CU_ASSERT_PTR_NULL(root->next);
   CU_ASSERT_EQUAL(((struct mock_list_node* )root)->data, nodeValue);
}

void create_list(struct mock_list_node** root, struct mock_list_node* nodesArr,
      int size)
{
   if (nodesArr != NULL && root != NULL)
   {
      for (int i = 0; i < size; i++)
      {
         nodesArr[i].data = i;
         append_node((struct list_node**) root,
               (struct list_node*) &nodesArr[i]);
      }
   }
   else
   {
      CU_FAIL("passed NULL pointer!");
   }
}

int list_iterate_null_parameter_mock_callback(struct list_node* node,
      void* uptr)
{
   if (uptr != NULL)
   {
      CU_FAIL("passed NULL pointer!");
   }
   else
   {
      CU_PASS("null parameter passed");
   }
   return 0;
}

int list_find_mock_callback(struct list_node* node, void* uptr)
{
   if (uptr == NULL || node == NULL)
   {
      CU_FAIL("passed NULL pointer!");
      return 0;
   }

   return ((struct mock_list_node*) node)->data - *((int*) uptr);
}

int list_iterate_not_called_mock_callback(struct list_node* node, void* uptr)
{
   CU_FAIL("Callback must not be called");
   return 0;
}

int list_iterate_mock_callback(struct list_node* node, void* uptr)
{
   if (uptr == NULL || node == NULL)
   {
      CU_FAIL("passed NULL pointer!");
      return 0;
   }

   ((struct mock_list_node*) node)->data = *((int*) uptr);

   return 0;
}
#endif /* LIST_TESTS_H_ */
