unit module support;

use NativeCall;
use MONKEY-SEE-NO-EVAL;

class P6Val is repr('CStruct') {
	has Pointer $.dummy1; #opaque.  Just has to be the right width
	has Pointer $.dummy2;
	has Pointer $.dummy3;
	has Pointer $.dummy4;
	has uint32 $.type;
}


my &make-nil = nativecast(:(--> P6Val), Pointer.new(+@*ARGS[2]));
my &make-int = nativecast(:(int64 --> P6Val), Pointer.new(+@*ARGS[3]));
my &make-num = nativecast(:(num64 --> P6Val), Pointer.new(+@*ARGS[4]));
my &make-str = nativecast(:(Str --> P6Val), Pointer.new(+@*ARGS[5]));
my &make-bool = nativecast(:(bool --> P6Val), Pointer.new(+@*ARGS[6]));
my &make-any = nativecast(:(Pointer --> P6Val), Pointer.new(+@*ARGS[7]));
my &make-new-error = nativecast(:(Str --> P6Val), Pointer.new(+@*ARGS[8]));
my &make-sub = nativecast(:(&fun (int64, int64), uint32, CArray[uint32], ssize_t --> P6Val), Pointer.new(+@*ARGS[9]));
my &make-list = nativecast(:(CArray[P6Val], size_t --> P6Val), Pointer.new(+@*ARGS[10]));
my &list-append = nativecast(:(P6Val, P6Val), Pointer.new(+@*ARGS[11]));

my &get-arity = nativecast(:(P6Val --> ssize_t), Pointer.new(+@*ARGS[12]));
my &get-bool = nativecast(:(P6Val --> bool), Pointer.new(+@*ARGS[13]));
my &get-funcptr = nativecast(:(P6Val --> Pointer), Pointer.new(+@*ARGS[14]));
my &get-int = nativecast(:(P6Val --> int64), Pointer.new(+@*ARGS[15]));
my &get-num = nativecast(:(P6Val --> num64), Pointer.new(+@*ARGS[16]));
my &get-parameter-types = nativecast(:(P6Val --> CArray[uint32]), Pointer.new(+@*ARGS[17]));
my &get-str = nativecast(:(P6Val --> Str), Pointer.new(+@*ARGS[18]));
my &list-index = nativecast(:(P6Val, size_t --> P6Val), Pointer.new(+@*ARGS[19]));
my &list-len = nativecast(:(P6Val --> size_t), Pointer.new(+@*ARGS[20]));
my &get-return-type = nativecast(:(P6Val --> uint32), Pointer.new(+@*ARGS[21]));

enum P6Type<P6Any P6Nil P6Int P6Num P6Str P6Bool P6Error P6Sub P6List>;
sub native-typeid-to-p6(uint32 $type) {
	given $type {
		when P6Any { return Any; }
		when P6Nil { return Any; }
		when P6Int { return int64; }
		when P6Num { return num64; }
		when P6Str { return Str; }
		when P6Bool { return bool; }
		when P6Sub { return Sub; }
		when P6List { return List; }
		default { return Any; }
	}
}
sub p6-typeid-to-native($type) {
	given ($type ~~ Parameter) ?? $type.type !! $type {
		when Bool { P6Bool; }
		when Int { P6Int; }
		when Num { P6Num; }
		when Str { P6Str; }
		when Sub { P6Sub; }
		when List { P6List; }
		when Nil { P6Nil; }
		default { P6Any; }
	}
}

sub p6-to-native($val --> P6Val) {
	given $val {
		when Bool { return make-bool $val; } # this has to be before Int, because Bool ~~ Int
		when Int { return make-int $val; }
		when Num { return make-num $val; }
		when Str { return make-str $val; }
		when Nil { return make-nil; }
		when Sub {
			# variadic
			if $val.arity != $val.count {
				return make-sub(nativecast(Pointer, $val), p6-typeid-to-native($val.signature.returns), Pointer.new, -1);
			} else {
				my $argtype-list = CArray[uint32].allocate($val.arity);
				for ^$val.arity {
					$argtype-list[$_] = p6-typeid-to-native $val.signature.params[$_];
				}
				return make-sub($val, p6-typeid-to-native($val.signature.returns), $argtype-list, $val.arity);
			}
		}
		when List|Range {
			my $ret = make-list CArray[P6Val].new, 0;
			for ^$val {
				list-append $ret, p6-to-native($val[$_]);
			}
			return $ret;
		}
		default { return make-any Pointer.new; }
	}
}
sub native-to-p6(P6Val $val) {
	given $val.type {
		when P6Any { return Any; }
		when P6Nil { return Nil; }
		when P6Int { return get-int $val; }
		when P6Num { return get-num $val; }
		when P6Str { return get-str $val; }
		when P6Bool { return get-bool $val; }

		when P6Sub {
			my @parameter-types;
			my $parameter-types = get-parameter-types $val;
			my $arity = get-arity $val;
			for ^$arity {
				@parameter-types.append: native-typeid-to-p6 $parameter-types[$_];
			}

			my $signature = Signature.new(params => @parameter-types.map({Parameter.new(type => $_)}), returns => native-typeid-to-p6 get-return-type $val); #, count => ($arity < 0) ?? Inf !! +@parameter-types);

			say "signature is {$signature.perl}, pointer is ", get-funcptr $val;
			return nativecast($signature, get-funcptr $val);
		}

		when P6List {
			my @ret;
			for ^(list-len $val) {
				@ret.append: native-to-p6 list-index($val, $_);
			}
			return @ret;
		}
		default { return Any; }
	}
}
sub evaluate(Str $x --> P6Val) {
	try {
		return p6-to-native EVAL $x;
	}

	if $! {
		return make-new-error $!.gist;
		return make-new-error($!.gist.lines[1] ~ " (" ~ $!.^name ~ "):\n" ~ $!.gist.lines[2 .. *].join("\n"));
	}
}
sub call(P6Val $fun, P6Val $args --> P6Val) {
	try {
		my &fun = native-to-p6 $fun;
		my @args = native-to-p6 $args;

		return p6-to-native(fun |@args);
	}

	if $! {
		return make-new-error $!.gist;
		return make-new-error($!.gist.lines[1] ~ " (" ~ $!.^name ~ "):\n" ~ $!.gist.lines[2 .. *].join("\n"));
	}
}
sub addering(int64 $a, int64 $b, int64 $c --> int64) {
	$a + $b + $c
}


my &set-evaluator = nativecast(:(Pointer, &callback (Str --> P6Val), &callback2 (P6Val, P6Val --> P6Val)), Pointer.new(+@*ARGS[0]));
&set-evaluator(Pointer.new(+@*ARGS[1]), &evaluate, &call);
