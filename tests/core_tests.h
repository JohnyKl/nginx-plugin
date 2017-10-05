#ifndef CORE_TESTS_H_
#define CORE_TESTS_H_

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include "../templarbit/core.h"

#include <curl/curl.h>

struct ngx_shm_zone
{
   int (*init)(void*, void*);
   void* data;
   void* shm;
};

static struct ngx_shm_zone mock_shm_zone;

static struct ngx_shm_zone* shmgetReturnValue = &mock_shm_zone;
static int shmatReturnValue = -1;
static int pthreadCreateReturnValue = -1;
static char* threadFunctionArgs = NULL;

void* mock_thread_function(void*);

/************* Test case functions prototypes ****************/

void test_post_handle_500_error(void);
void test_post_handle_broken_json(void);
void test_post_handle_no_csp_and_no_csp_ro(void);
void test_post_handle_timeout(void);
void test_process_server_instance_handle_exist(void);
void test_process_server_instance_handle_started(void);

/************* Test case functions ****************/

void test_post_handle_500_error(void)
{
	int result = REQ_OK;

	char* requestStr = const_cast<char*>("");
	char* requestUrl = const_cast<char*>("http://127.0.0.1:8080/test_500");
        char* responseStr = NULL;
        int responseCode;

	result = poll_api_impl(NULL, requestUrl, requestStr, 5, &responseStr, &responseCode);

	/* Assert */
	CU_ASSERT_EQUAL(result, REQ_FAILED);
}

void test_post_handle_broken_json(void)
{
	int result = REQ_OK;

	char* requestStr = const_cast<char*>("");
	char* requestUrl = const_cast<char*>("http://127.0.0.1:8080/test_bad_json");
        char* responseStr = NULL;
        int responseCode;

	result = poll_api_impl(NULL, requestUrl, requestStr, 5, &responseStr, &responseCode);
	/* Assert */
	CU_ASSERT_EQUAL(result, REQ_MALFORMED_RESPONSE);
}

void test_post_handle_no_csp_and_no_csp_ro(void)
{
	int result = REQ_OK;

	char* requestStr = const_cast<char*>("");
	char* requestUrl = const_cast<char*>("http://127.0.0.1:8080/test_no_csp");
        char* responseStr = NULL;
        int responseCode;

	result = poll_api_impl(NULL, requestUrl, requestStr, 5, &responseStr, &responseCode);

	/* Assert */
	CU_ASSERT_EQUAL(result, REQ_INVALID_RESPONSE);
}

void test_post_handle_timeout(void)
{
	int result = REQ_OK;

	char* requestStr = const_cast<char*>("");
	char* requestUrl = const_cast<char*>("http://127.0.0.1:8080/test_timeout");
        char* responseStr = NULL;
        int responseCode;

	result = poll_api_impl(NULL, requestUrl, requestStr, 5, &responseStr, &responseCode);

	/* Assert */
	CU_ASSERT_EQUAL(result, REQ_NO_RESPONSE);
}

void test_process_server_instance_handle_exist(void)
{
	struct handler_node handler;
	struct handler_node *handlers = &handler;
	char* token = (char*) "token";
	handler.token = token;
	char* propertyId = (char*) "1";
	void* serverConfig = NULL;

   int result = process_server_instance(NULL, &handlers, token, propertyId,
			serverConfig, NULL);

	CU_ASSERT_EQUAL(result, HANDLER_EXISTS);
}

void test_process_server_instance_shm_init_failed(void)
{
	struct handler_node *handlers = NULL;
	char* token = (char*) "token";
	char* propertyId = (char*) "1";
	void* serverConfig = NULL;
   shmgetReturnValue = 0;

   int result = process_server_instance(NULL, &handlers, token, propertyId,
			serverConfig, NULL);

	CU_ASSERT_EQUAL(result, HANDLER_SHM_INIT_FAILED);
}

void test_process_server_instance_handle_started(void)
{
	struct handler_node *handlers = NULL;
	char* token = (char*) "token";
	char* propertyId = (char*) "1";
	void* serverConfig = NULL;

	shmgetReturnValue = &mock_shm_zone;
	shmatReturnValue = 0;
	pthreadCreateReturnValue = 0;
	threadFunctionArgs = NULL;

	int result = process_server_instance(-1, &handlers, token, propertyId, serverConfig, NULL);
	struct handler_node* newHandler = handler_find_node(handlers, token);

	CU_ASSERT_EQUAL(result, HANDLER_STARTED);
	CU_ASSERT_PTR_NOT_NULL_FATAL(newHandler);
	CU_ASSERT_PTR_EQUAL(newHandler->clsv, serverConfig);
	CU_ASSERT_PTR_EQUAL(newHandler->csp_headers, shmatReturnValue);
}

struct ngx_shm_zone* mock_ngx_shared_memory_add(void *cf, void *name, size_t size, void *module)
{
	return shmgetReturnValue;
}
#endif /* CORE_TESTS_H_ */
