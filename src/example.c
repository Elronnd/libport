#include "port.h"
#include <stdio.h>

int main(void) {
	P6State *state1 = p6_init();
	//P6State *state2 = p6_init();
	// ^^ that breaks, for some reason?

	P6Val *v = p6eval(state1, "5 +");
	switch (v->type) {
		case P6Int: printf("%ld\n", v->integer); break;
		case P6Num: printf("%lf\n", v->num); break;
		case P6Str: printf("%s\n", v->str); break;
		case P6Bool: printf("%s\n", v->boolean ? "true" : "false"); break;
		case P6Error: printf("ERROR - %s\n", v->error_msg); break;
	}


	//p6_deinit(state2);
	p6_deinit(state1);
	return 0;
}
