#ifndef PORT_TESTING_H
#define PORT_TESTING_H
#include <stdbool.h>
#include <stdio.h>

bool in_failed_test;
char *test_name = NULL;
int num_passed_tests = 0;
int num_failed_tests = 0;

int num_passed_assertions = 0;
int num_failed_assertions = 0;

#define Assert(cond, descr_fmt, ...) do { \
	if (!(cond)) { \
		in_failed_test = true; \
		printf("\e[31mTEST FAILURE\e[0m in %s - ", test_name); \
		printf(descr_fmt, ## __VA_ARGS__); \
		printf("\n"); \
		num_failed_assertions++; \
	} else { \
		num_passed_assertions++; \
	} \
} while (0)

#define Test(new_test_name) do { \
	if (!test_name) { \
		/*pass*/ \
	} else if (in_failed_test) { \
		num_failed_tests++; \
		in_failed_test = false; \
	} else { \
		printf("\e[32mTEST SUCCESS\e[0m - %s\n", test_name); \
		num_passed_tests++; \
	} \
	test_name = new_test_name; \
} while (0)

#define End_Testing \
	if (in_failed_test) { \
		num_failed_tests++; \
	} else if (!in_failed_test) { \
		printf("\e[32mTEST SUCCESS\e[0m - %s\n", test_name); \
		num_passed_tests++; \
	} \
\
	printf("\n\n\n"); \
\
	printf("Passed %d/%d tests (%d/%d assertions)\n", num_passed_tests, num_passed_tests+num_failed_tests, num_passed_assertions, num_passed_assertions+num_failed_assertions); \
	return num_failed_tests;

#endif //PORT_TESTING_H
