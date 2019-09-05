#include "port.h"

int main(void) {
	P6State *state1 = p6_init();
	//P6State *state2 = p6_init();
	// ^^ that breaks, for some reason?

	p6eval(state1, "$scratch = 19");
	/*p6eval(state2, "$scratch = 22");
	 *p6eval(state2, "say $scratch");
	 */
	p6eval(state1, "say $scratch");

	//p6_deinit(state2);
	p6_deinit(state1);
	return 0;
}
