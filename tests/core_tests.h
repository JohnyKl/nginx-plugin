#ifndef CORE_TESTS_H_
#define CORE_TESTS_H_

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include "../templarbit/request.h"
#include "../poll_api_impl.h"

#include <curl/curl.h>

/************* Test case functions prototypes ****************/

void test_post_handle_500_error(void);
void test_post_handle_broken_json(void);
void test_post_handle_no_csp_and_no_csp_ro(void);
void test_post_handle_timeout(void);

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

#endif /* CORE_TESTS_H_ */
