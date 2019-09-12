unit module support;

use NativeCall;
use MONKEY-SEE-NO-EVAL;

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


my &make-nil = nativecast(:(--> Pointer[P6Val]), Pointer.new(+@*ARGS[2]));
my &make-int = nativecast(:(int64 --> Pointer[P6Val]), Pointer.new(+@*ARGS[3]));
my &make-num = nativecast(:(num64 --> Pointer[P6Val]), Pointer.new(+@*ARGS[4]));
my &make-str = nativecast(:(Str --> Pointer[P6Val]), Pointer.new(+@*ARGS[5]));
my &make-bool = nativecast(:(bool --> Pointer[P6Val]), Pointer.new(+@*ARGS[6]));
my &make-any = nativecast(:(Pointer --> Pointer[P6Val]), Pointer.new(+@*ARGS[7]));
my &make-new-error = nativecast(:(Str --> Pointer[P6Val]), Pointer.new(+@*ARGS[8]));

my $scratch;

sub evaluate(Str $x --> Pointer[P6Val]) {
	try {
		my $val = EVAL $x;
		given $val {
			when Bool { return make-bool $val; }
			when Int { return make-int $val; }
			when Num { return make-num $val; }
			when Str { return make-str $val; }
			when Nil { return make-nil; }
			default { return make-any Pointer.new(Pointer.new); }
		}
	}

	if $! {
		return make-new-error($!.gist.lines[1] ~ " (" ~ $!.^name ~ "):\n" ~ $!.gist.lines[2 .. *].join("\n"));
	}
}

my &set-evaluator = nativecast(:(Pointer, &callback (Str --> Pointer[P6Val])), Pointer.new(+@*ARGS[0]));
&set-evaluator(Pointer.new(+@*ARGS[1]), &evaluate);
