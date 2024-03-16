#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define ASSERTION_FAILURE_EXITCODE 47

#define register_test(f) register_test_internal(f, #f)
#define assert_int_equals(x, y) internal_assert_int_equals(__extension__ __FUNCTION__, __LINE__, #x, x, #y, y)
#define assert_dbl_equals(x, y) internal_assert_dbl_equals(__extension__ __FUNCTION__, __LINE__, #x, x, #y, y)
#define assert_str_equals(x, y) internal_assert_str_equals(__extension__ __FUNCTION__, __LINE__, #x, x, #y, y)
#define assert_ptr_equals(x, y) internal_assert_ptr_equals(__extension__ __FUNCTION__, __LINE__, #x, x, #y, y)
#define assert_mem_equals(x, y, l) internal_assert_mem_equals(__extension__ __FUNCTION__, __LINE__, #x, x, #y, y, l)
#define assert_that(x) internal_assert_that(__extension__ __FUNCTION__, __LINE__, #x, x)
#define assert_true(x) assert_that(x)
#define assert_false(x) assert_that(!(x))
#define assert_fail(x) internal_assert_fail(__extension__ __FUNCTION__, __LINE__, x)

void register_tests();
typedef void (*test_fp)();

typedef struct test_case {
    test_fp ptr;
    char *name;
    char *err;
    int passed;
    struct test_case *next;
} test_case;


#endif
