#ifndef HTTP_TESTS_H_
#define HTTP_TESTS_H_

#include "../templarbit/http.h"

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

#include <curl/curl.h>

/************* Test case functions prototypes ****************/

void test_make_http_request(void);
void test_GET_http_post_NULL_headers(void);
void test_GET_http_post_NOT_NULL_headers(void);
void test_check_request_headers(void);

/************* Test case functions ****************/

void test_make_http_request(void)
{
	http_request_t* result = NULL;

	char* requestBody = const_cast<char*>("");
	char* requestUrl = const_cast<char*>("http://127.0.0.1:8080/test_500");
	unsigned int timeout = 5;
	http_method_t method = http_method_t::GET;
	http_header_t* headers = NULL;

	result = make_http_request(requestUrl, requestBody, method, headers,
			timeout, timeout);

	/* Assert */
	CU_ASSERT_PTR_NOT_NULL_FATAL(result);
	CU_ASSERT_STRING_EQUAL(result->url, requestUrl);
	CU_ASSERT_STRING_EQUAL(result->request_body, requestBody);
	CU_ASSERT_EQUAL(result->connect_timeout, timeout);
	CU_ASSERT_EQUAL(result->response_timeout, timeout);
	CU_ASSERT_EQUAL(result->method, method);
	CU_ASSERT_PTR_EQUAL(result->headers, headers);
}

void test_GET_http_post_NULL_headers(void)
{
	http_response_t* response = NULL;

	unsigned int timeout = 5;
	http_method_t method = http_method_t::GET;
	http_header_t* headers = NULL;
	char* requestBody = const_cast<char*>("");
	char* requestUrl =
			const_cast<char*>("http://127.0.0.1:8080/test_get_method");

	http_request_t* request = make_http_request(requestUrl, requestBody, method,
			headers, timeout, timeout);

	response = http_post(request);

	CU_ASSERT_PTR_NOT_NULL_FATAL(response);
	CU_ASSERT_PTR_NOT_NULL_FATAL(response->response_body);
	CU_ASSERT_EQUAL(response->response_code, 200);

	if(response->response_code != 200)
	{
		printf("\nresponse->response_code = %i\n", response->response_code);
	}
}

void test_GET_http_post_NOT_NULL_headers(void)
{
	/* prepare */
	http_response_t* response = NULL;
	http_header_t* headers = NULL;
	header_append_node_n(&headers, "Content-Type: application/json");

	unsigned int timeout = 5;
	http_method_t method = http_method_t::GET;
	char* requestBody = const_cast<char*>("");
	char* requestUrl =
			const_cast<char*>("http://127.0.0.1:8080/test_get_method");

	http_request_t* request = make_http_request(requestUrl, requestBody, method,
			headers, timeout, timeout);

	/* call function */
	response = http_post(request);

	/*assert*/
	CU_ASSERT_PTR_NOT_NULL_FATAL(response);
	CU_ASSERT_PTR_NOT_NULL_FATAL(response->response_body);
	CU_ASSERT_EQUAL(response->response_code, 200);
}

void test_check_request_headers(void)
{
	/* prepare */
	http_response_t* response = NULL;
	http_header_t* headers = NULL;
	header_append_node_n(&headers, "Content-Type: application/json");

	unsigned int timeout = 5000;
	http_method_t method = http_method_t::GET;
	char* requestBody = const_cast<char*>("");
	char* requestUrl =
			const_cast<char*>("http://127.0.0.1:8080/test_check_headers");

	http_request_t* request = make_http_request(requestUrl, requestBody, method,
			headers, timeout, timeout);

	/* call function */
	response = http_post(request);

	printf(response->response_body);

	/*assert*/
	CU_ASSERT_PTR_NOT_NULL_FATAL(response);
	CU_ASSERT_PTR_NOT_NULL_FATAL(response->response_body);
	CU_ASSERT_EQUAL(response->response_code, 200);
	CU_ASSERT_PTR_NOT_NULL(strstr(response->response_body, "Content-Type: application/json"));
}

#endif /* HTTP_TESTS_H_ */
