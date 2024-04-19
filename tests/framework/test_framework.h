#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define ASSERTION_FAILURE_EXITCODE 47
#define SEQUENTIAL
typedef void (*test_fp)();

void register_test_internal(test_fp ptr, char *name);
void internal_assert_int_equals(const char *function_name, int line_number, const char *xname, int x, const char *yname, int y);
void internal_assert_int_equals(const char *function_name, int line_number, const char *xname, int x, const char *yname, int y);
void internal_assert_that(const char *function_name, int line_number, const char *condition_name, int condition);
void internal_assert_mem_equals(const char *function_name, int line_number, const char *xname, void *x, const char *yname, void *y, int length);
#define register_test(f) register_test_internal(f, #f)
#define assert_int_equals(x, y) internal_assert_int_equals(__extension__ __FUNCTION__, __LINE__, #x, x, #y, y)
#define assert_mem_equals(x, y, l) internal_assert_mem_equals(__extension__ __FUNCTION__, __LINE__, #x, x, #y, y, l)
#define assert_that(x) internal_assert_that(__extension__ __FUNCTION__, __LINE__, #x, x)
#define assert_true(x) assert_that(x)
#define assert_false(x) assert_that(!(x))
#define assert_fail(x) internal_assert_fail(__extension__ __FUNCTION__, __LINE__, x)

#include "../filesystem_api//fs_tests.h"
#include "../file_api/f_tests.h"


#endif
