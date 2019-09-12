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

P6Val *p6_make_new_error(char *msg);

struct P6State {
	MVMInstance *instance;
	MVMCompUnit *cu;
	P6Val *(*evaluator)(char *str);
};

static void toplevel_initial_invoke(MVMThreadContext *tc, void *data) {
	// Create initial frame, which sets up all of the interpreter state also.
	MVM_frame_invoke(tc, (MVMStaticFrame *)data, MVM_callsite_get_common(tc, MVM_CALLSITE_ID_NULL_ARGS), NULL, NULL, NULL, -1);
}

static void set_evaluator(P6State *state, P6Val *(*evaluator)(char*)) {
	state->evaluator = evaluator;
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

	char *raw_clargs[11];
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
	set_clarg(10, p6_make_new_error);

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
P6Val *p6_make_new_error(char *msg) {
	P6Val *ret = alloc(sizeof(P6Val));
	*ret = (P6Val){.type = P6Error, .error_msg = strdup(msg)};
	return ret;
}
