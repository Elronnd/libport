// originally based on Perl.xs @ https://github.com/niner/inline-perl6

#include <stdlib.h>
#ifdef __linux__
#include <alloca.h>
#endif

// moarvm headers have stuff that makes the compiler complain
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wtype-limits"
#include <moar.h>
#pragma GCC diagnostic pop

#define alloc(n) calloc(1, (n))

#define LIBPORT_IMPLEMENTATION
#include "port.h"

#include "fairy-juice.h"

struct P6State {
	MVMInstance *instance;
	MVMCompUnit *cu;
	P6Val *(*evaluator)(char *str);
	P6Val *(*caller)(P6Val *fun, P6Val *argumentlist);
};

static void toplevel_initial_invoke(MVMThreadContext *tc, void *data) {
	// Create initial frame, which sets up all of the interpreter state also.
	MVM_frame_invoke(tc, data, MVM_callsite_get_common(tc, MVM_CALLSITE_ID_NULL_ARGS), NULL, NULL, NULL, -1);
}

static void set_evaluator(P6State *state, P6Val *(*evaluator)(char*), P6Val *(*caller)(P6Val*,P6Val*)) {
	state->evaluator = evaluator;
	state->caller = caller;
}



LIBPORT_FUNC P6State *p6_init(void) {
	static const char *P6_MOAR_INTERPRETER = PERL6_INSTALL_PATH "/share/perl6/runtime/perl6.moarvm";

	P6State *ret = alloc(sizeof(P6State));
	if (!ret) {
		return NULL;
	}

	MVM_crash_on_error();

	ret->instance = MVM_vm_create_instance();

	ret->instance->lib_path[0] = PERL6_INSTALL_PATH "/share/nqp/lib";
	ret->instance->lib_path[1] = PERL6_INSTALL_PATH "/share/perl6/lib";
	ret->instance->lib_path[2] = PERL6_INSTALL_PATH "/share/perl6/runtime";
	ret->instance->lib_path[3] = NULL;

	// stash the rest of the raw command line args in the instance
	ret->instance->prog_name  = PERL6_INSTALL_PATH "/share/perl6/runtime/perl6.moarvm";
	ret->instance->exec_name  = PERL6_INSTALL_PATH "/bin/perl6-m";
	ret->instance->raw_clargs = NULL;

	// Map the compilation unit into memory and dissect it
	MVMThreadContext *tc = ret->instance->main_thread;
	ret->cu = MVM_cu_map_from_file(tc, P6_MOAR_INTERPRETER);

	MVMROOT(tc, ret->cu, {
			// The call to MVM_string_utf8_decode() may allocate, invalidating the location cu->body.filename
			MVMString *const str = MVM_string_utf8_decode(tc, ret->instance->VMString, P6_MOAR_INTERPRETER, strlen(P6_MOAR_INTERPRETER));
			ret->cu->body.filename = str;

			// Run deserialization frame, if there is one
			if (ret->cu->body.deserialize_frame) {
				MVM_interp_run(tc, &toplevel_initial_invoke, ret->cu->body.deserialize_frame);
			}
		});


#define set_clarg(n, p) \
	raw_clargs[n] = alloca(32); \
	sprintf(raw_clargs[n], "%zu", (uintptr_t)p)

	char *raw_clargs[24];
	ret->instance->num_clargs = sizeof(raw_clargs) / sizeof(raw_clargs[0]);

	raw_clargs[0] = "-e";
	raw_clargs[1] = MAGIC_FAIRY_JUICE;

	set_clarg(2, set_evaluator);
	set_clarg(3, ret);
	set_clarg(4, p6_make_nil);
	set_clarg(5, p6_make_int);
	set_clarg(6, p6_make_num);
	set_clarg(7, p6_make_str);
	set_clarg(8, p6_make_bool);
	set_clarg(9, p6_make_any);
	set_clarg(10, p6_make_error);
	set_clarg(11, p6_make_sub);
	set_clarg(12, p6_make_list);
	set_clarg(13, p6_list_append);

	set_clarg(14, p6_get_arity);
	set_clarg(15, p6_get_bool);
	set_clarg(16, p6_get_funcptr);
	set_clarg(17, p6_get_int);
	set_clarg(18, p6_get_num);
	set_clarg(19, p6_get_parameter_types);
	set_clarg(20, p6_get_str);
	set_clarg(21, p6_list_index);
	set_clarg(22, p6_list_len);
	set_clarg(23, p6_get_return_type);
#undef set_clarg

	ret->instance->raw_clargs = raw_clargs;
	ret->instance->clargs = NULL; // clear cache

	MVM_interp_run(tc, &toplevel_initial_invoke, ret->cu->body.main_frame);

	// Points to the current opcode
	MVMuint8 *cur_op = NULL;

	// The current frame's bytecode start
	MVMuint8 *bytecode_start = NULL;

	// Points to the base of the current register set for the frame we are presently in
	MVMRegister *reg_base = NULL;

	/* Stash addresses of current op, register base and SC deref base
	 * in the TC; this will be used by anything that needs to switch
	 * the current place we're interpreting.
	 */
	tc->interp_cur_op         = &cur_op;
	tc->interp_bytecode_start = &bytecode_start;
	tc->interp_reg_base       = &reg_base;
	tc->interp_cu             = &ret->cu;
	toplevel_initial_invoke(tc, ret->cu->body.main_frame);

	MVM_gc_mark_thread_blocked(tc);

	return ret;
}

LIBPORT_FUNC void p6_deinit(P6State *state) {
	if (state) free(state);

	// Disabled due to crashes. Also moarvm itself doesn't do this by default.
	//MVM_vm_destroy_instance(state->instance);
}

LIBPORT_FUNC P6Val *p6eval(P6State *state, char *text) {
	return state->evaluator(text);
}
LIBPORT_FUNC P6Val *p6call(P6State *state, P6Val *fun, P6Val *argumentlist) {
	assert (p6_typeof(fun) == P6Sub);
	assert (p6_typeof(argumentlist) == P6List);
	assert ((fun->sub.arity < 0) || (fun->sub.arity == argumentlist->list.len));
	return state->caller(fun, argumentlist);
}

LIBPORT_FUNC P6Val *p6_make_nil(void) {
	return NULL;
}
LIBPORT_FUNC P6Val *p6_make_any(void *any) {
	P6Val *ret = alloc(sizeof(P6Val));
	*ret = (P6Val){.type = P6Any, .any = any};
	return ret;
}
LIBPORT_FUNC P6Val *p6_make_int(int64_t integer) {
	P6Val *ret = alloc(sizeof(P6Val));
	*ret = (P6Val){.type = P6Int, .integer = integer};
	return ret;
}
LIBPORT_FUNC P6Val *p6_make_num(double num) {
	P6Val *ret = alloc(sizeof(P6Val));
	*ret = (P6Val){.type = P6Num, .num = num};
	return ret;
}
// N.B. need to strdup here.  Because str is from perl6-land and will get
// garbage-collected before long else.
LIBPORT_FUNC P6Val *p6_make_str(char *str) {
	P6Val *ret = alloc(sizeof(P6Val));
	*ret = (P6Val){.type = P6Str, .str = strdup(str)};
	return ret;
}
LIBPORT_FUNC P6Val *p6_make_bool(bool boolean) {
	P6Val *ret = alloc(sizeof(P6Val));
	*ret = (P6Val){.type = P6Bool, .boolean = boolean};
	return ret;
}
// ditto re strdup
P6Val *p6_make_error(char *msg) {
	P6Val *ret = alloc(sizeof(P6Val));
	*ret = (P6Val){.type = P6Error, .error_msg = strdup(msg)};
	return ret;
}
LIBPORT_FUNC P6Val *p6_make_sub(void *address, P6Type return_type, const P6Type *arguments, ssize_t arity) {
	P6Val *ret = alloc(sizeof(P6Val));
	ret->sub.addr = address;
	ret->sub.ret = return_type;
	ret->type = P6Sub;
	ret->sub.arity = arity;
	if (arity > 0) {
		ret->sub.arguments = alloc(sizeof(P6Type) * arity);
		memcpy(ret->sub.arguments, arguments, sizeof(P6Type) * arity);
	}

	return ret;
}
LIBPORT_FUNC P6Val *p6_make_list(const P6Val *vals, size_t num_vals) {
	P6Val *ret = alloc(sizeof(P6Val));
	ret->type = P6List;
	ret->list.len = num_vals;
	ret->list.vals = alloc(sizeof(P6Val) * num_vals);
	memcpy(ret->list.vals, vals, sizeof(P6Val) * num_vals);

	return ret;
}

LIBPORT_FUNC P6Type p6_typeof(const P6Val *val) {
	if (!val) return P6Nil;
	return val->type;
}
LIBPORT_FUNC void p6_val_free(P6Val *val) {
	switch (p6_typeof(val)) {
		// P6Nil is a NULL pointer, so can't access any members
		case P6Nil: return;

		// don't have any dynamic data
		case P6Any:
		case P6Int:
		case P6Num:
		case P6Bool:
			    break;

		case P6Str: free(val->str); break;
		case P6Error: free(val->error_msg); break;
		case P6Sub: free(val->sub.arguments); break;
		case P6List: free(val->list.vals); break;
	}

	free(val);
}
LIBPORT_FUNC P6Val *p6_val_copy(const P6Val *val) {
	switch (p6_typeof(val)) {
		case P6Str: return p6_make_str(val->str);
		case P6Error: return p6_make_error(val->error_msg);
		case P6Sub: return p6_make_sub(val->sub.addr, val->sub.ret, val->sub.arguments, val->sub.arity);
		case P6List: return p6_make_list(val->list.vals, val->list.len);

		case P6Any:
		case P6Int:
		case P6Num:
		case P6Bool:
			{
				P6Val *ret = alloc(sizeof(P6Val));
				*ret = *val;
				return ret;
			}

		case P6Nil: default: return NULL;

	}
}
LIBPORT_FUNC void p6_list_append(P6Val *list, P6Val *item) {
	assert (p6_typeof(list) == P6List); //TODO: figure out what the typechecking scheme should be

	list->list.vals = realloc(list->list.vals, sizeof(P6Val) * ++list->list.len);
	list->list.vals[list->list.len - 1] = *p6_val_copy(item);
}

LIBPORT_FUNC int64_t p6_get_int(const P6Val *val) {
	assert (p6_typeof(val) == P6Int);
	return val->integer;
}

LIBPORT_FUNC double p6_get_num(const P6Val *val) {
	assert (p6_typeof(val) == P6Num);
	return val->num;
}

LIBPORT_FUNC char *p6_get_str(const P6Val *val) {
	assert (p6_typeof(val) == P6Str);
	return val->str;
}

LIBPORT_FUNC bool p6_get_bool(const P6Val *val) {
	assert (p6_typeof(val) == P6Bool);
	return val->boolean;
}

LIBPORT_FUNC void *p6_get_funcptr(const P6Val *val) {
	assert (p6_typeof(val) == P6Sub);
	return val->sub.addr;
}

LIBPORT_FUNC ssize_t p6_get_arity(const P6Val *val) {
	assert (p6_typeof(val) == P6Sub);
	return val->sub.arity;
}

LIBPORT_FUNC P6Type p6_get_return_type(const P6Val *val) {
	assert (p6_typeof(val) == P6Sub);
	return val->sub.ret;
}

LIBPORT_FUNC P6Type *p6_get_parameter_types(const P6Val *val) {
	assert (p6_typeof(val) == P6Sub);
	return val->sub.arguments;
}

LIBPORT_FUNC P6Val *p6_list_index(const P6Val *val, size_t i) {
	assert (p6_typeof(val) == P6List);
	return val->list.vals + i;
}

LIBPORT_FUNC size_t p6_list_len(const P6Val *val) {
	assert (p6_typeof(val) == P6List);
	return val->list.len;
}

LIBPORT_FUNC bool p6_equal(const P6Val *lhs, const P6Val *rhs) {
	if (p6_typeof(lhs) != p6_typeof(rhs)) {
		return false;
	}

	switch (p6_typeof(lhs)) {
		case P6Any: case P6Nil: return true;
		case P6Int: return p6_get_int(lhs) == p6_get_int(rhs);
		case P6Num: return p6_get_num(lhs) == p6_get_num(rhs);
		case P6Str: return !strcmp(p6_get_str(lhs), p6_get_str(rhs));
		case P6Bool: return p6_get_bool(lhs) == p6_get_bool(rhs);
		case P6Error: return false;
		case P6Sub: return p6_get_funcptr(lhs) == p6_get_funcptr(rhs);
		case P6List:
			    if (p6_list_len(lhs) != p6_list_len(rhs)) {
				    return false;
			    }
			    for (size_t i = 0; i < p6_list_len(lhs); i++) {
				    if (!p6_equal(p6_list_index(lhs, i), p6_list_index(rhs, i))) {
					    return false;
				    }
			    }
			    return true;
	}

	assert(0);
}

LIBPORT_FUNC size_t p6_format_val_as_str(const P6Val *val, char *outbuf, size_t buflen) {
	if (!buflen) return 0;
#define proper_length (offset > buflen) ? 0 : buflen - offset

	size_t offset = 0;

	switch (p6_typeof(val)) {
		case P6Nil: return snprintf(outbuf, buflen, "Nil");
		case P6Any: return snprintf(outbuf, buflen, "<any %p>", val->any);
		case P6Int: return snprintf(outbuf, buflen, "%ld", val->integer);
		case P6Num: return snprintf(outbuf, buflen, "%lf", val->num);
		case P6Str: return snprintf(outbuf, buflen, "'%s'", val->str);
		case P6Bool: return snprintf(outbuf, buflen, "%s", val->boolean ? "True" : "False");
		case P6Error: return snprintf(outbuf, buflen, "ERROR - %s\n", val->error_msg);
		case P6List:
			offset += snprintf(outbuf, buflen, "[");
			for (size_t i = 0; i < val->list.len; i++) {
				offset += p6_format_val_as_str(val->list.vals + i, outbuf + offset, proper_length);
				if (i != val->list.len - 1) offset += snprintf(outbuf + offset, proper_length, " ");
			}
			return offset += snprintf(outbuf + offset, proper_length, "]");
		case P6Sub:
			offset += snprintf(outbuf, buflen, "<Sub %p>(", val->sub.addr);

			if (val->sub.arity < 0) {
				offset += snprintf(outbuf + offset, proper_length, "...");
			} else {
				for (size_t i = 0; i < val->sub.arity; i++) {
					offset += p6_format_type_as_str(val->sub.arguments[i], outbuf + offset, proper_length);
					if (i != val->sub.arity - 1) offset += snprintf(outbuf + offset, proper_length, ", ");
				}
			}

			offset += snprintf(outbuf + offset, proper_length, ") --> ");
			return offset += p6_format_type_as_str(val->sub.ret, outbuf + offset, proper_length);
			return offset += snprintf(outbuf + offset, proper_length, ">");
	}

	assert(0);
#undef proper_length
}

LIBPORT_FUNC size_t p6_format_type_as_str(P6Type type, char *outbuf, size_t buflen) {
	switch (type) {
		case P6Any: return snprintf(outbuf, buflen, "Any");
		case P6Nil: return snprintf(outbuf, buflen, "Nil");
		case P6Int: return snprintf(outbuf, buflen, "Int");
		case P6Num: return snprintf(outbuf, buflen, "Num");
		case P6Str: return snprintf(outbuf, buflen, "Str");
		case P6Bool: return snprintf(outbuf, buflen, "Bool");
		case P6Error: return snprintf(outbuf, buflen, "Error");
		case P6Sub: return snprintf(outbuf, buflen, "Sub");
		case P6List: return snprintf(outbuf, buflen, "List");
	}

	assert(0);
}
