use NativeCall;

say "I am a magic fairy ;o";
sub porter(int32) is native("./port") { ... }
say ";oo";

porter(7);
porter(8);
porter(9);
