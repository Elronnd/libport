unit module support;

use NativeCall;
use MONKEY-SEE-NO-EVAL;

my &make-none = nativecast(:(--> Pointer[P6Val]), Pointer.new(+@*ARGS[2]));
my &make-int = nativecast(:(int64 --> Pointer[P6Val]), Pointer.new(+@*ARGS[3]));
my &make-num = nativecast(:(num64 --> Pointer[P6Val]), Pointer.new(+@*ARGS[4]));
my &make-str = nativecast(:(Str --> Pointer[P6Val]), Pointer.new(+@*ARGS[5]));
my &make-bool = nativecast(:(bool --> Pointer[P6Val]), Pointer.new(+@*ARGS[6]));

class P6Data is repr('CUnion') {
	has int64 $.int;
	has num64 $.num;
	has Str $.str;
	has bool $.bool;
}
class P6Val is repr('CStruct') {
	has uint64 $.value; #opaque.  Just has to be the right width
	has uint8 $.type;
}


enum P6Type<type-none type-int type-num type-str type-bool>;

my $scratch;

sub evaluate(Str $x --> Pointer[P6Val]) {
	my $val = EVAL $x;
	given $val {
		when Int { return make-int $val; }
		when Num { return make-num $val; }
		when Str { return make-str $val; }
		when Bool { return make-bool $val; }
		default { return make-none; }
	}
}

my &set-evaluator = nativecast(:(Pointer, &callback (Str --> Pointer[P6Val])), Pointer.new(+@*ARGS[0]));
&set-evaluator(Pointer.new(+@*ARGS[1]), &evaluate);
