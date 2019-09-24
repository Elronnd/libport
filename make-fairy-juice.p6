my $text = 'static char * const MAGIC_FAIRY_JUICE = "';
$text ~= S:g/\"/\\\"/ given "src/support.p6".IO.lines.join("\\n\\\n");
$text ~= "\";\n";

# don't overwrite older version if it already exists
# this is just for make's dependency checking, which just looks at file modification times
exit if "src/fairy-juice.h".IO.e && $text eq slurp "src/fairy-juice.h";

spurt "src/fairy-juice.h", $text;
