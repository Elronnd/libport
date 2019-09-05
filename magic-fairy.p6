use NativeCall;

say "I am a magic fairy ;o";
sub id(size_t --> CArray[int32]) is native(IO::Path.new("port")) { ... }
my size_t $x = +@*ARGS[0];
my $p = id $x;
$p[0] = 17;


say ";oo";
