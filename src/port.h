#ifndef PORT_H
#define PORT_H

#ifdef _WIN32
# ifdef LIBPORT_IMPLEMENTATION
#  define LIBPORT_FUNC extern __declspec(dllexport)
# else
#  define LIBPORT_FUNC extern __declspec(dllimport)
# endif
#else
# define LIBPORT_FUNC extern
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct P6State P6State;

typedef enum {
	P6None,
	P6Int,
	P6Num,
	P6Str,
	P6Bool,
	//P6Error,
} P6Type;

typedef struct {
	union {
		int64_t integer;
		double num;
		char *str;
		bool boolean;
		//char *error_msg;
	};

	uint8_t type;
} P6Val;


LIBPORT_FUNC P6State *p6_init(void);
LIBPORT_FUNC void p6_deinit(P6State *state);
LIBPORT_FUNC P6Val p6eval(P6State *state, char *text);

#endif//PORT_H
