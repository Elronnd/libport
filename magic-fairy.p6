use NativeCall;

say "I am a magic fairy ;o";

use MONKEY-SEE-NO-EVAL;

sub evaluate(Str $x) { EVAL $x; }
sub id(size_t --> Pointer) is native(IO::Path.new("port")) { ... }
my size_t $x = +@*ARGS[0];
my $q = id $x;
my &p = nativecast(:(&callback (Str)), $q);
&p(&evaluate);

say ";oo";
