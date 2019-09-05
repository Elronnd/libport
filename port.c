#include <moar.h>

MVMInstance *instance;
MVMCompUnit *cu;

#define PERL6_INSTALL_PATH "/home/elronnd/.perl6install"
#define PERL6_EXECUTABLE "/home/elronnd/.perl6install/bin/perl6-m"
#define NQP_LIBDIR "/home/elronnd/.perl6install/share/nqp/lib"
#define mark_thread_blocked(tc) MVM_gc_mark_thread_blocked(tc)

const char *filename = PERL6_INSTALL_PATH "/share/perl6/runtime/perl6.moarvm";

static void toplevel_initial_invoke(MVMThreadContext *tc, void *data) {
    /* Create initial frame, which sets up all of the interpreter state also. */
    MVM_frame_invoke(tc, (MVMStaticFrame *)data, MVM_callsite_get_common(tc, MVM_CALLSITE_ID_NULL_ARGS), NULL, NULL, NULL, -1);
}

int exportme = 0;

void porter(int x) {
	printf("exported is %d and I have %d ;o\n", exportme, x);
	exportme++;
}

char *library_location = "/home/elronnd/perl5/lib/perl5/x86_64-linux-thread-multi/auto/Inline/Perl6/Perl6.so", *helper_path = "/home/elronnd/perl5/lib/perl5/x86_64-linux-thread-multi/Inline/Perl6/Helper.pm";

void initialize() {
        const char  *lib_path[8];
        const char *raw_clargs[2];

        int argi         = 1;
        int lib_path_i   = 0;

        MVM_crash_on_error();

        instance   = MVM_vm_create_instance();
        lib_path[lib_path_i++] = PERL6_INSTALL_PATH "/share/nqp/lib";
        lib_path[lib_path_i++] = PERL6_INSTALL_PATH "/share/perl6/lib";
        lib_path[lib_path_i++] = PERL6_INSTALL_PATH "/share/perl6/runtime";
        lib_path[lib_path_i++] = NQP_LIBDIR;
        lib_path[lib_path_i++] = NULL;

        for( argi = 0; argi < lib_path_i; argi++)
            instance->lib_path[argi] = lib_path[argi];

        /* stash the rest of the raw command line args in the instance */
        instance->prog_name  = PERL6_INSTALL_PATH "/share/perl6/runtime/perl6.moarvm";
        instance->exec_name  = PERL6_EXECUTABLE;
        instance->raw_clargs = NULL;

        /* Map the compilation unit into memory and dissect it. */
        MVMThreadContext *tc = instance->main_thread;
        cu = MVM_cu_map_from_file(tc, filename);

        MVMROOT(tc, cu, {
            /* The call to MVM_string_utf8_decode() may allocate, invalidating the
               location cu->body.filename */
            MVMString *const str = MVM_string_utf8_decode(tc, instance->VMString, filename, strlen(filename));
            cu->body.filename = str;

            /* Run deserialization frame, if there is one. */
            if (cu->body.deserialize_frame) {
                MVM_interp_run(tc, &toplevel_initial_invoke, cu->body.deserialize_frame);
            }
        });
        instance->num_clargs = 2;
        raw_clargs[0] = "magic-fairy.p6";
	static char buf[16];
	sprintf(buf, "%zu", &exportme);
	raw_clargs[1] = buf;


        instance->raw_clargs = (char **)raw_clargs;
        instance->clargs = NULL; /* clear cache */

        MVM_interp_run(tc, &toplevel_initial_invoke, cu->body.main_frame);

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
        tc->interp_cu             = &cu;
        toplevel_initial_invoke(tc, cu->body.main_frame);

        mark_thread_blocked(tc);
}

void p6_destroy() {
#if 0
	/* Disabled due to crashes. Also moarvm itself doesn't do this by default.
	   MVM_vm_destroy_instance(instance);
	   */
#endif
}

int main(void) {
	printf("I am maining \n");
	initialize();
	printf("I have mained %d", exportme);
}
