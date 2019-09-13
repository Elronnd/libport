unit module support;

use NativeCall;
use MONKEY-SEE-NO-EVAL;

class P6Val is repr('CStruct') {
	has Pointer $.dummy1; #opaque.  Just has to be the right width
	has Pointer $.dummy2;
	has Pointer $.dummy3;
	has Pointer $.dummy4;
}


my &make-nil = nativecast(:(--> Pointer[P6Val]), Pointer.new(+@*ARGS[2]));
my &make-int = nativecast(:(int64 --> Pointer[P6Val]), Pointer.new(+@*ARGS[3]));
my &make-num = nativecast(:(num64 --> Pointer[P6Val]), Pointer.new(+@*ARGS[4]));
my &make-str = nativecast(:(Str --> Pointer[P6Val]), Pointer.new(+@*ARGS[5]));
my &make-bool = nativecast(:(bool --> Pointer[P6Val]), Pointer.new(+@*ARGS[6]));
my &make-any = nativecast(:(Pointer --> Pointer[P6Val]), Pointer.new(+@*ARGS[7]));
my &make-new-error = nativecast(:(Str --> Pointer[P6Val]), Pointer.new(+@*ARGS[8]));
my &make-sub = nativecast(:(&fun, CArray[uint32], ssize_t --> Pointer[P6Val]), Pointer.new(+@*ARGS[9]));
my &make-list = nativecast(:(CArray[P6Val], size_t --> Pointer[P6Val]), Pointer.new(+@*ARGS[10]));
my &list-append = nativecast(:(Pointer[P6Val], Pointer[P6Val]), Pointer.new(+@*ARGS[11]));

sub p6-to-native($val --> Pointer[P6Val]) {
	given $val {
		when Bool { return make-bool $val; } # this has to be before Int, because Bool ~~ Int
		when Int { return make-int $val; }
		when Num { return make-num $val; }
		when Str { return make-str $val; }
		when Nil { return make-nil; }
		when Sub {
			return make-nil;
		}
		when List|Range {
			# TODO: why can't I do this?
			my $ret = make-list CArray[P6Val].new, 0;
			for ^$val {
				list-append $ret, p6-to-native($val[$_]);
			}
			return $ret;
		}
		default { return make-any Pointer.new; }
	}
}

sub evaluate(Str $x --> Pointer[P6Val]) {
	try {
		return p6-to-native EVAL $x;
	}

	if $! {
		return make-new-error $!.gist;
		return make-new-error($!.gist.lines[1] ~ " (" ~ $!.^name ~ "):\n" ~ $!.gist.lines[2 .. *].join("\n"));
	}
}

my &set-evaluator = nativecast(:(Pointer, &callback (Str --> Pointer[P6Val])), Pointer.new(+@*ARGS[0]));
&set-evaluator(Pointer.new(+@*ARGS[1]), &evaluate);
