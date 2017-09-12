#include "core.h"
#include "request.h"
#include "http.h"
#include "handler.h"
#include "../vendor/jansson/src/jansson.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>

#include <curl/curl.h>

#ifdef _TESTS
int mock_shmget(key_t key, size_t size, int shmflg);
int mock_shmat(int shmid, const void *shmaddr, int shmflg);
int mock_sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
int mock_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		void *(*start_routine)(void *), void *arg);

#define shmget(key, size, shmflg) mock_shmget(key, size, shmflg);
#define shmat(shmid, shmaddr, shmflg) mock_shmat(shmid, shmaddr, shmflg);
#define sem_open(name, oflag, mode, value) mock_sem_open(name, oflag, mode, value);
#define pthread_create(thread, attr, start_routine, arg) mock_pthread_create(thread, attr, start_routine, arg);
#endif

int poll_api_impl(struct handler_node *handler, char* url, char* request_body,
		int timeout)
{
	json_error_t json_error;
	json_t *root_node = NULL;
	int result = REQ_OK;
        const char *new_csp, *new_csp_ro;
        char *curr_csp, *curr_csp_ro;
	json_t *raw_csp, *raw_csp_ro;

	// constructing request struct
	http_header_t* headers = NULL;
	header_append_node_n(&headers, "Content-Type: application/json");
	http_request_t* request = make_http_request(url, request_body, POST,
			headers, timeout, timeout);

	// making request to templarbit server
	http_response_t* response = http_post(request);

	if (!response || !response->response_body
			|| response->curl_code == CURLE_OPERATION_TIMEDOUT)
	{
		result = REQ_NO_RESPONSE;
		goto finalize_request;
	}

	if (response->response_code != 200)
	{
		result = REQ_FAILED;
		goto finalize_request;
	}

	root_node = json_loads(response->response_body, 0, &json_error);
	if (!root_node)
	{
		result = REQ_MALFORMED_RESPONSE;
		goto finalize_request;
	}

	raw_csp = json_object_get(root_node, "csp");
        raw_csp_ro = json_object_get(root_node, "csp_report_only");

	if (!json_is_string(raw_csp) && !json_is_string(raw_csp_ro))
	{
		result = REQ_INVALID_RESPONSE;
		goto finalize_request;
	}

	new_csp = json_string_value(raw_csp);
        new_csp_ro = json_string_value(raw_csp_ro);

	curr_csp = handler->csp_headers->csp;
        curr_csp_ro = handler->csp_headers->csp_ro;

	if (strcmp(curr_csp, new_csp) // CSP are inequal
	|| strcmp(curr_csp_ro, new_csp_ro)) // CSP RO are inequal
	{
		// incrementing version number
		handler->csp_headers->version = handler->csp_headers->version + 1;

		// locking shared memory segment for update operation
		sem_wait(handler->csp_semid);

		if (new_csp)
		{
			memset(curr_csp, 0, CSP_HEADER_SIZE);
			strcpy(curr_csp, new_csp);
		}

		if (new_csp_ro)
		{
			memset(curr_csp_ro, 0, CSP_HEADER_SIZE);
			strcpy(curr_csp_ro, new_csp_ro);
		}

		// releasing shared memory segment lock
		sem_post(handler->csp_semid);

//       ngx_log_stderr(0, "Templarbit: New headers received. Content-Security-Policy: '%s', Content-Security-Policy-Report-Only: '%s'",
//          new_csp, new_csp_ro);
	}

	finalize_request: json_decref(root_node);
	free_http_request(request);
	free_http_response(response);

	return result;
}

int process_server_instance(struct handler_node **handlers, char* token,
		char *property_id, void* server_config, void*(*thread_function)(void*))
{
	pthread_t poller_thread; /* poller thread handler */
	int thread_status; /* poller thread creation status */
	struct handler_node *handler; /* templarbit server handler node */

	// checking if we have already handler/poller for the specific token
	handler = handler_find_node(*handlers, token);
	if (handler)
	{
		return HANDLER_EXISTS;
	}

	// preparing request body
	json_t *request = json_pack("{ssss}", "token", token, "property_id",
			property_id);

	handler = handler_append_node_n(handlers, token);
	handler->request_body = json_dumps(request, JSON_COMPACT);
	handler->clsv = server_config;

	// allocating shared memory segment for storing CSP header
	handler->csp_shmid = shmget(IPC_PRIVATE, CSP_SIZE, 0644 | IPC_CREAT);
	if (handler->csp_shmid == -1)
	{
		return HANDLER_SHM_INIT_FAILED;
	}

	handler->csp_headers = shmat(handler->csp_shmid, (void * )0, 0);
	if ((char*) handler->csp_headers == (char *) (-1))
	{
		return HANDLER_SHM_ATTACH_FAILED;
	}

	// creating semaphore in order to synchronize access to shared memory segment
	handler->csp_semid = sem_open(token, O_CREAT | O_EXCL, 0644, 1);

	// unlink prevents the semaphore existing forever
	sem_unlink(token);

	// starting CSP poller thread
	thread_status = pthread_create(&poller_thread, NULL, thread_function,
			token);
	if (thread_status != 0)
	{
		return HANDLER_THREAD_FAILED;
	}

	return HANDLER_STARTED;
}
