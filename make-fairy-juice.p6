my $text = 'static char * const MAGIC_FAIRY_JUICE = "';

for "src/support.p6".IO.lines {
	$text ~= "$_\\n\\\n";
}
$text ~= '";';

spurt "src/fairy-juice.h", $text;
