#include "port.h"
#include <stdio.h>

void print_p6val(P6Val *val) {
	switch (p6_typeof(val)) {
		case P6Nil: printf("Nil\n"); break;
		case P6Any: printf("<any %p>\n", val->any); break;
		case P6Int: printf("%ld\n", val->integer); break;
		case P6Num: printf("%lf\n", val->num); break;
		case P6Str: printf("%s\n", val->str); break;
		case P6Bool: printf("%s\n", val->boolean ? "true" : "false"); break;
		case P6Error: printf("ERROR - %s\n", val->error_msg); break;
		case P6List:
			printf("START list with length %zu\n", val->list.len);
			for (size_t i = 0; i < val->list.len; i++) {
				print_p6val(val->list.vals + i);
			}
			printf("END list\n");
			break;
		case P6Sub: printf("<Sub %p>\n", val->sub.addr); break;
		default: printf("Unknown type %u??\n", p6_typeof(val));
	}
}

int main(void) {
	P6State *state1 = p6_init();
	//P6State *state2 = p6_init();
	// ^^ that breaks, for some reason?
	// I'm fairly certain it's a moarvm bug; I've checked over and we don't
	// have anything important in static/global storage.

	P6Val *v = p6eval(state1, "[^5, 17, 'abc']");
	print_p6val(v);
	p6_val_free(v);


	//p6_deinit(state2);
	p6_deinit(state1);
	return 0;
}
