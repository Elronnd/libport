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

#define LIBPORT_IMPLEMENTATION
#include "port.h"

struct P6State {
	MVMInstance *instance;
	MVMCompUnit *cu;
	void (*evaluator)(char *str);
};

static void toplevel_initial_invoke(MVMThreadContext *tc, void *data) {
	// Create initial frame, which sets up all of the interpreter state also.
	MVM_frame_invoke(tc, (MVMStaticFrame *)data, MVM_callsite_get_common(tc, MVM_CALLSITE_ID_NULL_ARGS), NULL, NULL, NULL, -1);
}

static void set_evaluator(P6State *state, void(*evaluator)(char*)) {
	state->evaluator = evaluator;
}


static char * const MAGIC_FAIRY_JUICE = "use NativeCall;\
use MONKEY-SEE-NO-EVAL;\
my $scratch = 17;\
sub evaluate(Str $x) { EVAL $x; };\
my &set-evaluator = nativecast(:(Pointer, &callback (Str)), Pointer.new(+@*ARGS[0]));\
&set-evaluator(Pointer.new(+@*ARGS[1]), &evaluate);";

LIBPORT_FUNC P6State *p6_init(void) {
	static const char *P6_MOAR_INTERPRETER = PERL6_INSTALL_PATH "/share/perl6/runtime/perl6.moarvm";

	P6State *ret = calloc(1, sizeof(P6State));
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

	ret->instance->num_clargs = 4;

	char *raw_clargs[4];
	raw_clargs[0] = "-e";
	raw_clargs[1] = MAGIC_FAIRY_JUICE;

	raw_clargs[2] = alloca(32);
	sprintf(raw_clargs[2], "%zu", (uintptr_t)set_evaluator);

	raw_clargs[3] = alloca(32);
	sprintf(raw_clargs[3], "%zu", (uintptr_t)ret);


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

LIBPORT_FUNC void p6eval(P6State *state, char *text) {
	state->evaluator(text);
}
