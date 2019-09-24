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
			printf("[");
			for (size_t i = 0; i < val->list.len; i++) {
				print_p6val(val->list.vals + i);
			}
			printf("]\n");
			break;
		case P6Sub:
			printf("<Sub %p>", val->sub.addr);

			printf("(");
			if (val->sub.arity < 0) {
				printf("...");
			} else {
				for (size_t i = 0; i < val->sub.arity; i++) {
					switch (val->sub.arguments[i]) {
						case P6Any: printf("Any"); break;
						case P6Nil: printf("Nil"); break;
						case P6Int: printf("Int"); break;
						case P6Num: printf("Num"); break;
						case P6Str: printf("Str"); break;
						case P6Bool: printf("Bool"); break;
						case P6Error: printf("Error"); break;
						case P6Sub: printf("Sub"); break;
						case P6List: printf("List"); break;
					}
					if (i != val->sub.arity - 1) printf(", ");
				}
			}

			printf(")\n");
			break;
		default: printf("Unknown type %u??\n", p6_typeof(val));
	}
}

int main(void) {
	P6State *state1 = p6_init();
	//P6State *state2 = p6_init();
	// ^^ that breaks, for some reason?
	// I'm fairly certain it's a moarvm bug; I've checked over and we don't
	// have anything important in static/global storage.

	P6Val *fun = p6eval(state1, "&addering");
	print_p6val(fun);
	P6Val *arglist = p6_make_list(NULL, 0);
	p6_list_append(arglist, p6_make_int(17));
	p6_list_append(arglist, p6_make_int(14));
	p6_list_append(arglist, p6_make_int(33));

	P6Val *returned = p6call(state1, fun, arglist);
	print_p6val(returned);
	p6_val_free(returned);
	p6_val_free(arglist);
	p6_val_free(fun);


	//p6_deinit(state2);
	p6_deinit(state1);
	return 0;
}
