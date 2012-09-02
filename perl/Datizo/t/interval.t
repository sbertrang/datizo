
use strict;
use warnings;

use Test::More
    tests => 1
;

BEGIN {
    use_ok( 'Datizo' );
}

my $string = "5000 hours 3 seconds";

my $span = Datizo::Interval->new( $string );

use Devel::Peek;

Dump $span;

warn "Datizo::Interval->new( $string ) = $span\n";

my $now = Datizo::TimestampTz->now();

warn "now: $now\n";

my $then = $now->add( $span );

warn "then: $then ($now+$span)\n";



my $lifetime = Datizo::Interval->new( "1000000 hours" );

warn "lifetime->justify: " . $lifetime->justify() . "\n";
warn "lifetime->justify(hours): " . $lifetime->justify('hours') . "\n";
warn "lifetime->justify(days): " . $lifetime->justify('days') . "\n";


my $endoflife = $now->add( $lifetime );

warn "endoflife: $endoflife ($now+$lifetime)\n";

warn "now 123 years ago: " . Datizo::TimestampTz->now->minus( Datizo::Interval->new('123y') );
