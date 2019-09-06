use NativeCall;
use MONKEY-SEE-NO-EVAL;

class P6Var is repr('CUnion') {
	has int64 $.int;
	has num64 $.num;
	has Str $.str;
	has bool $.boolean;
}
class P6Val is repr('CStruct') {
	HAS P6Var $.value;
	has int8 $.type;
}

my $scratch;

sub evaluate(Str $x) { EVAL $x; };
my &set-evaluator = nativecast(:(Pointer, &callback (Str)), Pointer.new(+@*ARGS[0]));
&set-evaluator(Pointer.new(+@*ARGS[1]), &evaluate);
