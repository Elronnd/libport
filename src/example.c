#include "port.h"
#include <stdio.h>

int main(void) {
	P6State *state1 = p6_init();
	//P6State *state2 = p6_init();
	// ^^ that breaks, for some reason?

	P6Val *v = p6eval(state1, "5+5e0");
	printf("returned type %d val %lf\n", v->type, v->num);

	//p6_deinit(state2);
	p6_deinit(state1);
	return 0;
}
