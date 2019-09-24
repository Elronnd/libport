#include "port.h"
#include "port-testing.h"
#include <string.h>

int main(void) {
	P6Val *val;

	Test("Initialization");
	P6State *state = p6_init();
	Assert(state, "Failed to initialise");

	Test("Numbers");
	val = p6eval(state, "+'1'");
	Assert(p6_typeof(val) == P6Int, "type is %d but should be P6Int (%d)", p6_typeof(val), P6Int);
	Assert(p6_get_int(val) == 1, "value is %ld but should be 1", p6_get_int(val));

	val = p6eval(state, "15.78");
	Assert(p6_typeof(val) == P6Num, "type is %d but should be P6Num (%d)", p6_typeof(val), P6Num);
	Assert(p6_get_num(val) == 15.78, "value is %lf, but should be 15.78", p6_get_num(val));

	val = p6eval(state, "12 + 14.4");
	Assert(p6_typeof(val) == P6Num, "type is %d but should be P6Num (%d)", p6_typeof(val), P6Num);
	Assert(p6_get_num(val) == 26.4, "value is %lf, but should be 26.4", p6_get_num(val));

	Test("Strings");
	val = p6eval(state, "~18");
	Assert(p6_typeof(val) == P6Str, "type is %d but should be P6Str (%d)", p6_typeof(val), P6Str);
	Assert(!strcmp(p6_get_str(val), "18"), "value is '%s', but should be 18", p6_get_str(val));

	Test("Nil/Any");
	val = p6eval(state, "Nil");
	Assert(p6_typeof(val) == P6Nil, "type is %d but should be P6Nil (%d)", p6_typeof(val), P6Nil);
	val = p6eval(state, "Any");
	Assert(p6_typeof(val) == P6Any, "type is %d but should be P6Any (%d)", p6_typeof(val), P6Any);

	Test("Error");
	val = p6eval(state, "~~~");
	Assert(p6_typeof(val) == P6Error, "type is %d but should be P6Error (%d)", p6_typeof(val), P6Error);

	Test("Bool");
	val = p6eval(state, "?24");
	Assert(p6_typeof(val) == P6Bool, "type is %d but should be P6Bool (%d)", p6_typeof(val), P6Bool);
	Assert(p6_get_bool(val) == true, "value is false, but should be true");

	val = p6eval(state, "!24");
	Assert(p6_typeof(val) == P6Bool, "type is %d but should be P6Bool (%d)", p6_typeof(val), P6Bool);
	Assert(p6_get_bool(val) == false, "value is true, but should be false");

	Test("List");
	val = p6eval(state, "[16, ^3, 'h']");
	Assert(p6_typeof(val) == P6List, "type is %d but should be P6List (%d)", p6_typeof(val), P6List);
	Assert(p6_list_len(val) == 3, "length is %zu but should be 3", p6_list_len(val));

	Assert(p6_typeof(p6_list_index(val, 0)) == P6Int, "type of first item is %d but should be P6Int (%d)", p6_typeof(p6_list_index(val, 0)), P6Int);
	val = p6_list_index(val, 1);
	Assert(p6_typeof(val) == P6List, "type of second item is %d but should be P6List (%d)", p6_typeof(val), P6List);
	Assert(p6_list_len(val) == 3, "length of second val should be 3 but is %zu", p6_list_len(val));
	Assert(p6_typeof(p6_list_index(val, 1)) == P6Int, "second item should contain P6Int (%d), but instead contains a mysterious (%d)", P6Int, p6_typeof(p6_list_index(val, 1)));
	Assert(p6_get_int(p6_list_index(val, 1)) == 1, "should have been 1, was %ld", p6_get_int(p6_list_index(val, 1)));

	End_Testing;
}
