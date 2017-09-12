#include <stdio.h> 

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

#include "server/HttpMockServer.h"
#include "process_server_instance_tests.h"
#include "core_tests.h"
#include "http_tests.h"
#include "list_tests.h"
#include "handler_tests.h"

/* Test Suite setup and cleanup functions: */

int init_suite(void)
{
	return 0;
}
int clean_suite(void)
{
	return 0;
}

int main(void)
{
//	load_library();

	/* Run the server */
	Server serv;
	serv.start();

	CU_pSuite pPollApiImplSuite = NULL;
	CU_pSuite pListSuite = NULL;
	CU_pSuite pHttpSuite = NULL;
	CU_pSuite pHandlerSuite = NULL;
	CU_pSuite pProcServInstSuite = NULL;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* add a suite to the registry */
	pPollApiImplSuite = CU_add_suite("poll_api_impl_test_suite", init_suite,
			clean_suite);
	pHttpSuite = CU_add_suite("http_test_suite", init_suite, clean_suite);
	pListSuite = CU_add_suite("list_test_suite", init_suite, clean_suite);
	pHandlerSuite = CU_add_suite("handler_test_suite", init_suite, clean_suite);
	pProcServInstSuite = CU_add_suite("process_server_instance_test_suite",
			init_suite, clean_suite);

	if ( NULL == pPollApiImplSuite || NULL == pListSuite || NULL == pHttpSuite
			|| NULL == pHandlerSuite || NULL == pProcServInstSuite)
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* add the tests to the suite */
	if ((CU_add_test(pPollApiImplSuite, "test_post_handle_500_error",
			test_post_handle_500_error) == NULL)
			|| (CU_add_test(pPollApiImplSuite, "test_post_handle_broken_json",
					test_post_handle_broken_json) == NULL)
			|| (CU_add_test(pPollApiImplSuite,
					"test_post_handle_no_csp_and_no_csp_ro",
					test_post_handle_no_csp_and_no_csp_ro) == NULL)
			|| (CU_add_test(pPollApiImplSuite, "test_post_handle_timeout",
					test_post_handle_timeout) == NULL)
			|| (CU_add_test(pPollApiImplSuite, "test_make_http_request",
					test_make_http_request) == NULL)
			|| (CU_add_test(pHttpSuite, "test_GET_http_post_NULL_headers",
					test_GET_http_post_NULL_headers) == NULL)
			|| (CU_add_test(pHttpSuite, "test_GET_http_post_NOT_NULL_headers",
					test_GET_http_post_NOT_NULL_headers) == NULL)
			|| (CU_add_test(pHttpSuite, "test_check_request_headers",
					test_check_request_headers) == NULL)
			|| (CU_add_test(pListSuite, "test_list_append_null_node",
					test_list_append_null_node) == NULL)
			|| (CU_add_test(pListSuite, "test_list_append_null_node_head",
					test_list_append_null_node_head) == NULL)
			|| (CU_add_test(pListSuite, "test_list_append_node_head",
					test_list_append_node_head) == NULL)
			|| (CU_add_test(pListSuite, "test_list_append_node_second",
					test_list_append_node_second) == NULL)
			|| (CU_add_test(pListSuite, "test_list_append_node_tail",
					test_list_append_node_tail) == NULL)
			|| (CU_add_test(pListSuite, "test_list_find_exist_node",
					test_list_find_exist_node) == NULL)
			|| (CU_add_test(pListSuite, "test_list_find_not_exist_node",
					test_list_find_not_exist_node) == NULL)
			|| (CU_add_test(pListSuite, "test_list_find_null_parameters",
					test_list_find_null_parameters) == NULL)
			|| (CU_add_test(pListSuite, "test_list_iterate", test_list_iterate)
					== NULL)
			|| (CU_add_test(pListSuite, "test_list_iterate_null_parameters",
					test_list_iterate_null_parameters) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_find_node_null_params",
					test_handler_find_node_null_params) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_find_node_exist_token",
					test_handler_find_node_exist_token) == NULL)
			|| (CU_add_test(pListSuite,
					"test_handler_find_node_exist_token_dup",
					test_handler_find_node_exist_token_dup) == NULL)
			|| (CU_add_test(pListSuite,
					"test_handler_find_node_not_exist_token",
					test_handler_find_node_not_exist_token) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_append_null_node_head",
					test_handler_append_null_node_head) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_append_null_node",
					test_handler_append_null_node) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_append_node_head",
					test_handler_append_node_head) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_append_node_second",
					test_handler_append_node_second) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_append_node_tail",
					test_handler_append_node_tail) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_append_n_node_head",
					test_handler_append_n_node_head) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_append_n_node_second",
					test_handler_append_n_node_second) == NULL)
			|| (CU_add_test(pListSuite, "test_handler_append_n_node_tail",
					test_handler_append_n_node_tail) == NULL)
			|| (CU_add_test(pProcServInstSuite,
					"test_process_server_instance_handle_exist",
					test_process_server_instance_handle_exist) == NULL)
			|| (CU_add_test(pProcServInstSuite,
					"test_process_server_instance_shm_init_failed",
					test_process_server_instance_shm_init_failed) == NULL)
			|| (CU_add_test(pProcServInstSuite,
					"test_process_server_instance_shm_attach_failed",
					test_process_server_instance_shm_attach_failed) == NULL)
			|| (CU_add_test(pProcServInstSuite,
					"test_process_server_instance_handle_thread_failed",
					test_process_server_instance_handle_thread_failed) == NULL)
			|| (CU_add_test(pProcServInstSuite,
					"test_process_server_instance_handle_started",
					test_process_server_instance_handle_started) == NULL))

	{
		CU_cleanup_registry();
		return CU_get_error();
	}
// Run all tests using the basic interface
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	printf("\n");
	CU_basic_show_failures(CU_get_failure_list());
	printf("\n\n");
	/*
	 // Run all tests using the automated interface
	 CU_automated_run_tests();
	 CU_list_tests_to_file();

	 // Run all tests using the console interface
	 CU_console_run_tests();
	 */
	/* Clean up registry and return */
	CU_cleanup_registry();

	return CU_get_error();
}
