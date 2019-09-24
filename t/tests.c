#include "port.h"
#include "port-testing.h"
#include <string.h>

int main(void) {
	P6Val *val;

	Test("Initialization");
	P6State *state = p6_init();
	Assert(state, "Failed to initialise");

	Test("Numbers");
	AssertEq(p6eval(state, "+'1'"), p6_make_int(1));
	AssertEq(p6eval(state, "15.78"), p6_make_num(15.78));
	AssertEq(p6eval(state, "12 + 14.4"), p6_make_num(26.4));

	Test("Strings");
	AssertEq(p6eval(state, "~18"), p6_make_str("18"));

	Test("Nil/Any");
	AssertIsType(p6eval(state, "Nil"), P6Nil);
	AssertIsType(p6eval(state, "Any"), P6Any);

	Test("Error");
	AssertIsType(p6eval(state, "~~~"), P6Error);

	Test("Bool");
	AssertEq(p6eval(state, "?24"), p6_make_bool(true));
	AssertEq(p6eval(state, "!24"), p6_make_bool(false));

	Test("List");
	val = p6eval(state, "[16, ^3, 'h']");
	AssertIsType(val, P6List);
	Assert(p6_list_len(val) == 3, "length is %zu but should be 3", p6_list_len(val));

	AssertIsType(p6_list_index(val, 0), P6Int);
	val = p6_list_index(val, 1);
	AssertIsType(val, P6List);
	Assert(p6_list_len(val) == 3, "length of second val should be 3 but is %zu", p6_list_len(val));
	AssertIsType(p6_list_index(val, 1), P6Int);
	AssertEq(p6_list_index(val, 1), p6_make_int(1));

	Test("Sub");
	val = p6eval(state, "&addering");
	AssertIsType(val, P6Sub);
	Assert(p6_get_arity(val) == 3, "arity is %zi but should be 3", p6_get_arity(val));
	Assert(p6_get_return_type(val) == P6Int, "return type is %d but should be P6Int (%d)", p6_get_return_type(val), P6Int);
	Assert(p6_get_parameter_types(val)[1] == P6Int, "type of 2nd parameter is %d; should be P6Int (%d)", p6_get_parameter_types(val)[1], P6Int);

	End_Testing;
}
