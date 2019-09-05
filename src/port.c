#include <moar.h>

#include "port.h"

#define PERL6_INSTALL_PATH "/home/elronnd/.perl6install"
#define NQP_LIBDIR "/home/elronnd/.perl6install/share/nqp/lib"
const char *filename = PERL6_INSTALL_PATH "/share/perl6/runtime/perl6.moarvm";

struct P6State {
	MVMInstance *instance;
	MVMCompUnit *cu;
	void (*evaluator)(char *str);
};

static void toplevel_initial_invoke(MVMThreadContext *tc, void *data) {
	// Create initial frame, which sets up all of the interpreter state also.
	MVM_frame_invoke(tc, (MVMStaticFrame *)data, MVM_callsite_get_common(tc, MVM_CALLSITE_ID_NULL_ARGS), NULL, NULL, NULL, -1);
}

void set_evaluator(P6State *state, void(*evaluator)(char*)) {
	state->evaluator = evaluator;
}

P6State *p6_init(void) {
	P6State *ret = malloc(sizeof(P6State));
	if (!ret) {
		return NULL;
	}

	const char  *lib_path[8];
	const char *raw_clargs[3];

	size_t lib_path_i = 0;

	MVM_crash_on_error();

	ret->instance = MVM_vm_create_instance();
	lib_path[lib_path_i++] = PERL6_INSTALL_PATH "/share/nqp/lib";
	lib_path[lib_path_i++] = PERL6_INSTALL_PATH "/share/perl6/lib";
	lib_path[lib_path_i++] = PERL6_INSTALL_PATH "/share/perl6/runtime";
	lib_path[lib_path_i++] = NQP_LIBDIR;
	lib_path[lib_path_i++] = NULL;

	for (size_t argi = 0; argi < lib_path_i; argi++)
		ret->instance->lib_path[argi] = lib_path[argi];

	// stash the rest of the raw command line args in the instance
	ret->instance->prog_name  = PERL6_INSTALL_PATH "/share/perl6/runtime/perl6.moarvm";
	ret->instance->exec_name  = "/home/elronnd/.perl6install/bin/perl6-m";
	ret->instance->raw_clargs = NULL;

	/* Map the compilation unit into memory and dissect it. */
	MVMThreadContext *tc = ret->instance->main_thread;
	ret->cu = MVM_cu_map_from_file(tc, filename);

	MVMROOT(tc, ret->cu, {
			/* The call to MVM_string_utf8_decode() may allocate, invalidating the
			   location cu->body.filename */
			MVMString *const str = MVM_string_utf8_decode(tc, ret->instance->VMString, filename, strlen(filename));
			ret->cu->body.filename = str;

			/* Run deserialization frame, if there is one. */
			if (ret->cu->body.deserialize_frame) {
				MVM_interp_run(tc, &toplevel_initial_invoke, ret->cu->body.deserialize_frame);
			}
		});

	ret->instance->num_clargs = 3;
	raw_clargs[0] = "magic-fairy.p6";
	static char buf[30];
	static char buf2[30];
	sprintf(buf, "%zu", &set_evaluator);
	sprintf(buf2, "%zu", ret);
	raw_clargs[1] = buf;
	raw_clargs[2] = buf2;


	ret->instance->raw_clargs = (char **)raw_clargs;
	//ret->instance->clargs = NULL; /* clear cache */

	MVM_interp_run(tc, &toplevel_initial_invoke, ret->cu->body.main_frame);

	/* Points to the current opcode. */
	MVMuint8 *cur_op = NULL;

	/* The current frame's bytecode start. */
	MVMuint8 *bytecode_start = NULL;

	/* Points to the base of the current register set for the frame we
	 * are presently in. */
	MVMRegister *reg_base = NULL;

	/* Stash addresses of current op, register base and SC deref base
	 * in the TC; this will be used by anything that needs to switch
	 * the current place we're interpreting. */
	tc->interp_cur_op         = &cur_op;
	tc->interp_bytecode_start = &bytecode_start;
	tc->interp_reg_base       = &reg_base;
	tc->interp_cu             = &ret->cu;
	toplevel_initial_invoke(tc, ret->cu->body.main_frame);

	MVM_gc_mark_thread_blocked(tc);

	return ret;
}

void p6_deinit(P6State *state) {
	free(state);
	// Disabled due to crashes. Also moarvm itself doesn't do this by default.
	//MVM_vm_destroy_instance(state->instance);
}

void p6eval(P6State *state, char *text) {
	state->evaluator(text);
}
