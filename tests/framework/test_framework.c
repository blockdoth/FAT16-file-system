#include "../fs_tests.h"
#include "test_framework.h"


typedef struct test_case {
    test_fp ptr;
    char *name;
    char *err;
    int passed;
    struct test_case *next;
} test_case;


test_case *test_cases = NULL;

void register_test_internal(test_fp ptr, char *name) {
    test_case *tc = malloc(sizeof(test_case));
    tc->ptr = ptr;
    tc->name = strdup(name);
    tc->passed = -1;
    tc->next = test_cases;
    test_cases = tc;
}

void unregister_tests() {
    while (test_cases != NULL) {
        test_case *cur = test_cases;
        test_cases = cur->next;
        free(cur->name);
        free(cur);
    }
}

void run_test_case(test_case *tc) {
    #ifndef SEQUENTIAL
    fflush(stdout);		// such that Valgrind does not print first
    pid_t pid = fork();

    if (pid == 0) {
        (tc->ptr)();
        unregister_tests();
        exit(0);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        tc-> passed = 1;
    }
    else {
        tc->passed = 0;
    }
    #else
    tc->ptr();
    tc->passed = 1;
    #endif

}

void run_suite() {
    test_case *tc = test_cases;
    while (tc != NULL) {
        run_test_case(tc);
        tc = tc->next;
    }
}

void show_results() {
    int total_score = 0;
    int achieved_score = 0;

    printf("\n[Tests]\n");
    for (test_case *cur = test_cases; cur != NULL; cur = cur->next) {
        total_score++;
        if (cur->passed <= 0) {
            printf("Test case failed: '%s'\n", cur->name);
        } else {
            printf("Test case passed: '%s'\n", cur->name);
            achieved_score++;
        }
    }
    printf("\n[Results]\n");

    printf("Tests passed: %d/%d\n", achieved_score, total_score);
}

void internal_assert_int_equals(const char *function_name, int line_number, const char *xname, int x, const char *yname, int y) {
    if (x != y) {
        fprintf(stderr, "%d. %s: Expected <%s>:%d to be equal to <%s>:%d.\n", line_number, function_name, xname, x, yname, y);
        exit(ASSERTION_FAILURE_EXITCODE);
    }
}

void internal_assert_dbl_equals(const char *function_name, int line_number, const char *xname, double x, const char *yname, double y) {
    if (x != y) {
        fprintf(stderr, "%d. %s: Expected <%s>:%f to be equal to <%s>:%f.\n", line_number, function_name, xname, x, yname, y);
        exit(ASSERTION_FAILURE_EXITCODE);
    }
}

void internal_assert_ptr_equals(const char *function_name, int line_number, const char *xname, void *x, const char *yname, void *y) {
    if (x != y) {
        fprintf(stderr, "%d. %s: Expected <%s>:%p to be equal to <%s>:%p.\n", line_number, function_name, xname, x, yname, y);
        exit(ASSERTION_FAILURE_EXITCODE);
    }
}

void internal_assert_str_equals(const char *function_name, int line_number, const char *xname, char *x, const char *yname, char *y) {
    if (strcmp(x, y) != 0) {
        fprintf(stderr, "%d. %s: Expected <%s>:\"%s\" to be equal to <%s>:\"%s\".\n", line_number, function_name, xname, x, yname, y);
        exit(ASSERTION_FAILURE_EXITCODE);
    }
}

void internal_assert_that(const char *function_name, int line_number, const char *condition_name, int condition) {
    if (!condition) {
        fprintf(stderr, "%d. %s: Expected '%s' to hold.\n", line_number, function_name, condition_name);
        exit(ASSERTION_FAILURE_EXITCODE);
    }
}

void internal_assert_mem_equals(const char *function_name, int line_number, const char *xname, void *x, const char *yname, void *y, int length) {
    if (memcmp(x, y, length) != 0) {
        fprintf(stderr, "%d. %s: Expected memory at <%s>:%p to be equal to <%s>:%p.\n", line_number, function_name, xname, x, yname, y);
        exit(ASSERTION_FAILURE_EXITCODE);
    }
}

void internal_assert_fail(const char *function_name, int line_number, char *message) {
    fprintf(stderr, "%d. %s: %s.\n", line_number, function_name, message);
    exit(ASSERTION_FAILURE_EXITCODE);
}


int main(int argc, char *argv[]) {
    register_tests();
    run_suite();
    show_results();
    unregister_tests();
    return 0;
}


