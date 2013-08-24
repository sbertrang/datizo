
use strict;
use warnings;

use Test::More
    tests => 1
;

BEGIN {
    use_ok( 'Datizo' );
}

my @dates = qw(
	1999/9/9
	2012/12/12
	2010/10/10
	2011/11/11
);

my @timestamps = map Datizo::TimestampTz->new( $_ ), @dates;

for my $timestamp ( sort @timestamps ) {
	warn( "timestamp: $timestamp\n" );
}
