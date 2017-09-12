#ifndef HANDLER_TESTS_H_
#define HANDLER_TESTS_H_

#include "../templarbit/handler.h"

#ifndef TEST_NODES_ARRAY_NODES_COUNT
#define TEST_NODES_ARRAY_NODES_COUNT 6
#endif

void create_handlers_list(struct handler_node** root,
		struct handler_node* handlersArray, int size);

/************* Test case functions prototypes ****************/

void test_handler_find_node_null_params(void);
void test_handler_find_node_exist_token(void);
void test_handler_find_node_exist_token_dup(void);
void test_handler_find_node_not_exist_token(void);

void test_handler_append_null_node_head(void);
void test_handler_append_null_node(void);
void test_handler_append_node_head(void);
void test_handler_append_node_second(void);
void test_handler_append_node_tail(void);

void test_handler_append_n_node_head(void);
void test_handler_append_n_node_second(void);
void test_handler_append_n_node_tail(void);

/************* Test case functions ****************/

void test_handler_find_node_null_params(void)
{
	struct handler_node node;
	struct handler_node* root = &node;
	char* token = (char*) "invalid token";

	struct handler_node* nodeNullRoot = handler_find_node(NULL, token);
	struct handler_node* nodeNullToken = handler_find_node(root, NULL);

	CU_ASSERT_PTR_NULL(nodeNullRoot);
	CU_ASSERT_PTR_NULL(nodeNullToken);
}

void test_handler_find_node_exist_token(void)
{
	char* token = (char*) "token";
	struct handler_node nodeArr[TEST_NODES_ARRAY_NODES_COUNT];
	struct handler_node* root = NULL;
	create_handlers_list(&root, nodeArr, TEST_NODES_ARRAY_NODES_COUNT);
	nodeArr[TEST_NODES_ARRAY_NODES_COUNT / 2].token = token;

	struct handler_node* result = handler_find_node(root, token);

	CU_ASSERT_PTR_EQUAL(result, &nodeArr[TEST_NODES_ARRAY_NODES_COUNT / 2]);
}

void test_handler_find_node_not_exist_token(void)
{
	char* token = (char*) "not exist token";
	struct handler_node nodeArr[TEST_NODES_ARRAY_NODES_COUNT];
	struct handler_node* root = NULL;
	create_handlers_list(&root, nodeArr, TEST_NODES_ARRAY_NODES_COUNT);

	struct handler_node* result = handler_find_node(root, token);

	CU_ASSERT_PTR_NULL(result);
}

void test_handler_find_node_exist_token_dup(void)
{
	char* token = (char*) "token";
	struct handler_node nodeArr[TEST_NODES_ARRAY_NODES_COUNT];
	struct handler_node* root = NULL;
	create_handlers_list(&root, nodeArr, TEST_NODES_ARRAY_NODES_COUNT);
	nodeArr[TEST_NODES_ARRAY_NODES_COUNT / 2].token = token;
	nodeArr[TEST_NODES_ARRAY_NODES_COUNT / 2 + 1].token = token;

	struct handler_node* result = handler_find_node(root, token);

	CU_ASSERT_PTR_EQUAL(result, &nodeArr[TEST_NODES_ARRAY_NODES_COUNT / 2]);
}

void test_handler_append_null_node_head(void)
{
	struct handler_node* root = NULL;

	handler_append_node(&root, NULL);

	CU_ASSERT_PTR_NULL(root);
}

void test_handler_append_null_node(void)
{
	struct handler_node* root = NULL;
	struct handler_node newNode;
	newNode.next = &newNode; // check if we do nothing with list while append NULL node pointer
	root = &newNode;

	handler_append_node(&root, NULL);

	CU_ASSERT_PTR_NOT_NULL_FATAL(root);
	CU_ASSERT_PTR_EQUAL(root, &newNode);
	CU_ASSERT_PTR_EQUAL(root->next, &newNode);
	CU_ASSERT_PTR_EQUAL(newNode.next, &newNode);
}

void test_handler_append_node_head(void)
{
	struct handler_node* root = NULL;
	struct handler_node newNode;
	newNode.next = &newNode; // check if pointer to the next node became NULL for the tail node

	handler_append_node(&root, &newNode);

	CU_ASSERT_PTR_NOT_NULL_FATAL(root);
	CU_ASSERT_PTR_EQUAL(root, &newNode);
	CU_ASSERT_PTR_NULL(root->next);
	CU_ASSERT_PTR_NULL(newNode.next);
}

void test_handler_append_node_second(void)
{
	struct handler_node* root = NULL;
	struct handler_node newNode1, newNode2;
	memset(&newNode1, 0, sizeof(struct handler_node));
	memset(&newNode2, 0, sizeof(struct handler_node));

	root = &newNode1;
	newNode1.next = NULL;

	handler_append_node(&root, &newNode2);

	CU_ASSERT_PTR_NOT_NULL_FATAL(root);
	CU_ASSERT_PTR_EQUAL(root, &newNode1);
	CU_ASSERT_PTR_EQUAL(root->next, &newNode2);
	CU_ASSERT_PTR_EQUAL(newNode1.next, &newNode2);
	CU_ASSERT_PTR_NULL(root->next->next);
	CU_ASSERT_PTR_NULL(newNode2.next);
}

void test_handler_append_node_tail(void)
{
	struct handler_node* root = NULL;
	struct handler_node newNode1, newNode2, newNode3;
	memset(&newNode1, 0, sizeof(struct handler_node));
	memset(&newNode2, 0, sizeof(struct handler_node));
	memset(&newNode3, 0, sizeof(struct handler_node));

	root = &newNode1;
	newNode1.next = &newNode2;

	handler_append_node(&root, &newNode3);

	CU_ASSERT_PTR_NOT_NULL_FATAL(root);
	CU_ASSERT_PTR_EQUAL(root, &newNode1);

	CU_ASSERT_PTR_EQUAL(root->next, &newNode2);
	CU_ASSERT_PTR_EQUAL(newNode1.next, &newNode2);

	CU_ASSERT_PTR_EQUAL(root->next->next, &newNode3);
	CU_ASSERT_PTR_EQUAL(newNode2.next, &newNode3);

	CU_ASSERT_PTR_NULL(root->next->next->next);
	CU_ASSERT_PTR_NULL(newNode3.next);
}

void test_handler_append_n_node_head(void)
{
	struct handler_node* root = NULL;
	char* newToken = "newToken";

	struct handler_node* newNode = handler_append_node_n(&root, newToken);

	CU_ASSERT_PTR_NOT_NULL_FATAL(root);
	CU_ASSERT_STRING_EQUAL(root->token, newToken);
	CU_ASSERT_PTR_EQUAL(root, newNode);
	CU_ASSERT_PTR_NULL(root->next);
	CU_ASSERT_PTR_NULL(root->request_body);
}

void test_handler_append_n_node_second(void)
{
	struct handler_node* root = NULL;
	struct handler_node newNode1;
	char* newToken1 = "newToken1";
	char* newToken2 = "newToken2";
	memset(&newNode1, 0, sizeof(struct handler_node));
	newNode1.token = newToken1;
	root = &newNode1;
	newNode1.next = NULL;

	struct handler_node* newNode = handler_append_node_n(&root, newToken2);

	CU_ASSERT_PTR_NOT_NULL_FATAL(root);
	CU_ASSERT_PTR_NOT_NULL_FATAL(root->next);

	CU_ASSERT_PTR_EQUAL(root, &newNode1);
	CU_ASSERT_PTR_EQUAL(root->next, newNode);
	CU_ASSERT_STRING_EQUAL(root->token, newToken1);
	CU_ASSERT_STRING_EQUAL(root->next->token, newToken2);
	CU_ASSERT_PTR_NULL(root->next->next);
}

void test_handler_append_n_node_tail(void)
{
	char* token = (char*) "newToken";
	struct handler_node nodeArr[TEST_NODES_ARRAY_NODES_COUNT];
	struct handler_node* root = NULL;
	create_handlers_list(&root, nodeArr, TEST_NODES_ARRAY_NODES_COUNT);

	struct handler_node* newNode = handler_append_node_n(&root, token);

	CU_ASSERT_PTR_NOT_NULL_FATAL(newNode);
	CU_ASSERT_PTR_EQUAL(nodeArr[TEST_NODES_ARRAY_NODES_COUNT - 1].next,
			newNode);
	CU_ASSERT_STRING_EQUAL(newNode->token, token);
}

void create_handlers_list(struct handler_node** root,
		struct handler_node* handlersArray, int size)
{
	char* defaultToken = (char*) "default_token_val";

	if (handlersArray != NULL && root != NULL)
	{
		for (int i = 0; i < size; i++)
		{
			memset(&handlersArray[i], 0, sizeof(struct handler_node));

			handlersArray[i].csp_shmid = i;
			handlersArray[i].token = defaultToken;

			append_node((struct list_node**) root,
					(struct list_node*) &handlersArray[i]);
		}
	}
	else
	{
		CU_FAIL("passed NULL pointer!");
	}
}

#endif /* HANDLER_TESTS_H_ */
