use NativeCall;

say "I am a magic fairy ;o";

use MONKEY-SEE-NO-EVAL;

sub evaluate(Str $x) { EVAL $x; }
my &set-evaluator = nativecast(:(Pointer, &callback (Str)), Pointer.new(+@*ARGS[0]));
&set-evaluator(Pointer.new(+@*ARGS[1]), &evaluate);

say ";oo";
