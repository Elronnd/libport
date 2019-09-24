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
		printf("\e[31mTEST FAILURE\e[0m in \e[4m%s\e[0m \e[34m(line %d)\e[0m: ", test_name, __LINE__); \
		printf(descr_fmt, ## __VA_ARGS__); \
		printf("\n"); \
		num_failed_assertions++; \
	} else { \
		num_passed_assertions++; \
	} \
} while (0)


#define AssertEq(lhs, rhs) do { \
	char _tmp1[64], _tmp2[64]; \
	p6_format_val_as_str(lhs, _tmp1, sizeof(_tmp1)); \
	p6_format_val_as_str(rhs, _tmp2, sizeof(_tmp2)); \
	Assert(p6_equal(lhs, rhs), "\e[36m%s\e[0m != \e[35m%s\e[0m", _tmp1, _tmp2); \
} while (0)

#define AssertIsType(val, type) do { \
	char _tmp1[64], _tmp2[64]; \
	p6_format_val_as_str(val, _tmp1, sizeof(_tmp1)); \
	p6_format_type_as_str(type, _tmp2, sizeof(_tmp2)); \
	Assert(p6_typeof(val) == type, "\e[36m%s\e[0m !~~ \e[33m%s\e[0m", _tmp1, _tmp2); \
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
	printf("\n\n"); \
\
	printf("Passed %d/%d tests (%d/%d assertions).\n", num_passed_tests, num_passed_tests+num_failed_tests, num_passed_assertions, num_passed_assertions+num_failed_assertions); \
	return num_failed_tests;

#endif //PORT_TESTING_H
