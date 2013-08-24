
use strict;
use warnings;

use Test::More
    tests => 1
;

BEGIN {
    use_ok( 'Datizo' );
}

my $dtz = Datizo::TimestampTz->new("Sun Sep  9 10:33:00 CEST 2012");


warn "scalar object: $dtz\n";

