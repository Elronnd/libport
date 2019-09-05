#include "port.h"

int main(void) {
	P6State *state = p6_init();

	p6eval(state, "say 5;");
	p6eval(state, "say 5;");
	p6eval(state, "say 5;");

	p6_deinit(state);
	return 0;
}
