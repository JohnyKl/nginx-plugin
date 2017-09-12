#ifndef PROCESS_SERVER_INSTANCE_TESTS_H_
#define PROCESS_SERVER_INSTANCE_TESTS_H_

#include "../poll_api_impl.h"

static int shmgetReturnValue = -1;
static int shmatReturnValue = -1;
static int pthreadCreateReturnValue = -1;
static char* threadFunctionArgs = NULL;

void* mock_thread_function(void*);

/************* Test case functions prototypes ****************/

void test_process_server_instance_handle_exist(void);
void test_process_server_instance_shm_init_failed(void);
void test_process_server_instance_shm_attach_failed(void);
void test_process_server_instance_handle_thread_failed(void);
void test_process_server_instance_handle_started(void);

/************* Test case functions ****************/

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

	CU_ASSERT_EQUAL(result, HANDLER_STARTED);
	CU_ASSERT_STRING_EQUAL(threadFunctionArgs,token);
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
	threadFunctionArgs = args;
	return NULL;
}

#endif /* PROCESS_SERVER_INSTANCE_TESTS_H_ */
