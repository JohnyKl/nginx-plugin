#ifndef CORE_TESTS_H_
#define CORE_TESTS_H_

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include "../templarbit/request.h"
#include "../templarbit/core.h"

#include <curl/curl.h>

static int shmgetReturnValue = -1;
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
void test_process_server_instance_shm_init_failed(void);
void test_process_server_instance_shm_attach_failed(void);
void test_process_server_instance_handle_thread_failed(void);
void test_process_server_instance_handle_started(void);

/************* Test case functions ****************/

void test_post_handle_500_error(void)
{
	int result = REQ_OK;

	char* requestStr = const_cast<char*>("");
	char* requestUrl = const_cast<char*>("http://127.0.0.1:8080/test_500");

	result = poll_api_impl(NULL, requestUrl, requestStr, 5);

	/* Assert */
	CU_ASSERT_EQUAL(result, REQ_FAILED);
}

void test_post_handle_broken_json(void)
{
	int result = REQ_OK;

	char* requestStr = const_cast<char*>("");
	char* requestUrl = const_cast<char*>("http://127.0.0.1:8080/test_bad_json");

	result = poll_api_impl(NULL, requestUrl, requestStr, 5);
	/* Assert */
	CU_ASSERT_EQUAL(result, REQ_MALFORMED_RESPONSE);
}

void test_post_handle_no_csp_and_no_csp_ro(void)
{
	int result = REQ_OK;

	char* requestStr = const_cast<char*>("");
	char* requestUrl = const_cast<char*>("http://127.0.0.1:8080/test_no_csp");

	result = poll_api_impl(NULL, requestUrl, requestStr, 5);

	/* Assert */
	CU_ASSERT_EQUAL(result, REQ_INVALID_RESPONSE);
}

void test_post_handle_timeout(void)
{
	int result = REQ_OK;

	char* requestStr = const_cast<char*>("");
	char* requestUrl = const_cast<char*>("http://127.0.0.1:8080/test_timeout");

	result = poll_api_impl(NULL, requestUrl, requestStr, 5);

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

	int result = process_server_instance(&handlers, token, propertyId,
			serverConfig, NULL);

	CU_ASSERT_EQUAL(result, HANDLER_EXISTS);
}

void test_process_server_instance_shm_init_failed(void)
{
	struct handler_node *handlers = NULL;
	char* token = (char*) "token";
	char* propertyId = (char*) "1";
	void* serverConfig = NULL;
	shmgetReturnValue = -1;

	int result = process_server_instance(&handlers, token, propertyId,
			serverConfig, NULL);

	CU_ASSERT_EQUAL(result, HANDLER_SHM_INIT_FAILED);
}

void test_process_server_instance_shm_attach_failed(void)
{
	struct handler_node *handlers = NULL;
	char* token = (char*) "token";
	char* propertyId = (char*) "1";
	void* serverConfig = NULL;
	shmgetReturnValue = 0;
	shmatReturnValue = -1;

	int result = process_server_instance(&handlers, token, propertyId,
			serverConfig, NULL);

	CU_ASSERT_EQUAL(result, HANDLER_SHM_ATTACH_FAILED);
}

void test_process_server_instance_handle_thread_failed(void)
{
	struct handler_node *handlers = NULL;
	char* token = (char*) "token";
	char* propertyId = (char*) "1";
	void* serverConfig = NULL;
	shmgetReturnValue = 0;
	shmatReturnValue = 0;
	pthreadCreateReturnValue = -1;

	int result = process_server_instance(&handlers, token, propertyId,
			serverConfig, NULL);

	CU_ASSERT_EQUAL(result, HANDLER_THREAD_FAILED);
}

void test_process_server_instance_handle_started(void)
{
	struct handler_node *handlers = NULL;
	char* token = (char*) "token";
	char* propertyId = (char*) "1";
	void* serverConfig = NULL;
	shmgetReturnValue = 0;
	shmatReturnValue = 0;
	pthreadCreateReturnValue = 0;
	threadFunctionArgs = NULL;

	int result = process_server_instance(&handlers, token, propertyId,
			serverConfig, mock_thread_function);
	struct handler_node* newHandler = handler_find_node(handlers, token);

	CU_ASSERT_EQUAL(result, HANDLER_STARTED);
	CU_ASSERT_PTR_NOT_NULL_FATAL(newHandler);
	CU_ASSERT_PTR_EQUAL(newHandler->clsv, serverConfig);
	CU_ASSERT_EQUAL(newHandler->csp_shmid, shmgetReturnValue);
	CU_ASSERT_PTR_EQUAL(newHandler->csp_headers, shmatReturnValue);
	CU_ASSERT_PTR_EQUAL(newHandler->csp_semid, 0);
}

int mock_shmget(key_t key, size_t size, int shmflg)
{
	return shmgetReturnValue;
}

int mock_shmat(int shmid, const void *shmaddr, int shmflg)
{
	return shmatReturnValue;
}

int mock_sem_open(const char *name, int oflag, mode_t mode, unsigned int value)
{
	return 0;
}

int mock_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		void *(*start_routine)(void *), void *arg)
{
	if(pthreadCreateReturnValue == 0)
	{
		return pthread_create(thread, attr, start_routine, arg);
	}

	return pthreadCreateReturnValue;
}

void* mock_thread_function(void* args)
{
	if(strcmp("token", (char*)args) != 0)
	{
		CU_FAIL("Passed wrong argument to mock_thread_function");
	}
	else
	{
		CU_PASS("mock_thread_function ran successful");
	}
	return NULL;
}

#endif /* CORE_TESTS_H_ */
