#ifndef TESTS_SERVER_H_
#define TESTS_SERVER_H_

#include <iostream>
#ifndef _MSC_VER
#include <pthread.h>
#include <unistd.h>
#else
#include <windows.h>
#include <iostream>

typedef void* pthread_t;
#endif

class Server
{
private:
	static int gtcnt;
	pthread_t thread;
	int id;

	static void * _thread_start(void* instance)
	{
		Server *casted = ((Server *) instance);
		std::cout << "Thread #" << casted->getId() << " starting" << std::endl;
		casted->run();
		return NULL;
	}

	void run();

public:
	const char* route_500_error = "/test_500";
	const char* route_bad_json = "/test_bad_json";
	const char* route_timeout = "/test_timeout";
	const char* route_no_csp_no_csp_ro = "/test_no_csp";
	const char* route_get_method = "/test_get_method";
	const char* route_check_headers = "/test_check_headers";

	int getId()
	{
		return id;
	}

	Server()
	{
		id = gtcnt++;
		thread = 0;
	}

	void join()
	{
#ifdef _MSC_VER
		WaitForSingleObject(thread, INFINITE);
#else
		pthread_join(thread, NULL);
#endif
	}

	void start()
	{
#ifdef _MSC_VER
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_thread_start, this, 0, NULL);
#else
		pthread_create(&thread, NULL, _thread_start, this);
#endif
	}
};

#endif /* TESTS_SERVER_H_ */
