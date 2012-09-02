
use strict;
use warnings;

use Test::More
    tests => 1
;

BEGIN {
    use_ok( 'Datizo' );
}

my $dtz = Datizo::TimestampTz->new("2012/6/6 10:24");

use Devel::Peek;

Dump $dtz;

warn "scalar object: $dtz\n";

my $now = Datizo::TimestampTz->now();

warn "now: $now\n";

my $tz = $now->timezone("GMT");

warn("now: $now ($tz)");

