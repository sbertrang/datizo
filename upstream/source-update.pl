#!/usr/bin/perl

#
# scan for source comments, then check, compare and eventually update
#

use strict;
use warnings;
use Cwd qw( realpath );
use Data::Dumper qw( Dumper );

sub cleanargs
{
	my $string = shift;

	# fold multiple spaces into one
	$string =~ s! \s+ ! !gmsx;

	# remove leading space
	$string =~ s!\A[ ]!!x;
	# remove trailing
	$string =~ s![ ]\z!!x;

	# adjust spaces in struct names
	$string =~ s! [*][ ] !*!gmsx;

	# adjust for local struct change
	$string =~ s!\b struct[ ]pg_tm \b!struct tm!gx;

	#warn( "arg string: <$string>" );

	return $string;
}

sub anonvar
{
	my $var = shift;

	$var =~ s! \s+ ! !gmsx;
	$var =~ s!\A[ ]!!x;
	$var =~ s![ ]\z!!x;

	# last element is a variable
	$var =~ s!\A ( .+? ) (?: ([ ][*]) | [ ] ) [a-z0-9_]+? \z!$1.($2||'')!ex;

	return $var;
}

my $libdir = realpath( "../lib" );
my $distdir = realpath( "./postgresql-9.2.2" );
my @libsrcs = map( m!\A $libdir / (.+) \z!x ? $1 : (), glob( "$libdir/*.c" ) );
my %libsrcs;
my %libdists;
my %distlibs;

for my $libsrc ( @libsrcs ) {
	if ( open( my $fh, '<', "$libdir/$libsrc" ) ) {
		local( $/ );
		$libsrcs{ $libsrc } = <$fh>;
		close( $fh );
	}
	else {
		warn( "$!: $libsrc" );
		next;
	}

	unless ( $libsrcs{ $libsrc } =~ m! ^/[*][ ]postgresql:[ ](.+)[ ][*]/\n !x ) {
		warn( "no source comment found: $libsrc\n" );
		next;
	}
	my $distsrc = $1;

	$libdists{ $libsrc } = $distsrc;

	push( @{ $distlibs{ $distsrc } }, $libsrc );
}

my @distsrcs = sort( keys( %distlibs ) );
my %distsrcs;


warn( Dumper( {
	'distlibs'	=> \%distlibs,
#	'libdists'	=> \%libdists,
	'distsrcs'	=> \@distsrcs,
} ) );

# load dist sources

for my $distsrc ( @distsrcs ) {
	if ( open( my $fh, '<', "$distdir/$distsrc" ) ) {
		local( $/ );
		$distsrcs{ $distsrc } = <$fh>;
		close( $fh );
	}
	else {
		warn( "$!: $distsrc" );
		next;
	}

}

my $refuncret = "([\\w\\d\\s_*-]+)";
my $refuncargs = "\\( \\s* (.*?) \\s* \\)";
my $refuncbody = "( \\n \\{ \\n .*? \\n \\} \\n )";

my @libfuncs;
my %libfuncs;

for my $libsrc ( @libsrcs ) {
	my ( $func ) = $libsrc =~ m!\A (.+) [.] c \z!x;

	push( @libfuncs, $func );

	if ( $libsrcs{ $libsrc } =~ m! ^ $refuncret $func $refuncargs $refuncbody !msx ) {
		my $ret = $1;
		my $args = $2;
		my $code = $3;
		warn( "found old function: $ret - $func ( $args )\n$code\n\n" );

		$libfuncs{ $func } = [ cleanargs( $ret ), cleanargs( $args ), $code ];
	}
	else {
		warn( "could not find old function: $func\n" );
	}
}

my $relibfuncs = join( "|", map( quotemeta, @libfuncs ) );

my %distfuncs;

for my $distsrc ( @distsrcs ) {
	while ( $distsrcs{ $distsrc } =~ m! ^ $refuncret ($relibfuncs) $refuncargs $refuncbody !gmsx ) {
		my $ret = $1;
		my $func = $2;
		my $code = $4;
		my $args = $3;
		warn( "found new function: $ret - $func ( $args )\n$code\n\n" );

		$distfuncs{ $func } = [ cleanargs( $ret ), cleanargs( $args ), $code ];
	}
}

warn( "compare results...\n" );

# map postgresql macros to real types
my %typemap;
my @later;

for my $name ( @libfuncs ) {
	my $old = $libfuncs{ $name };
	my $new = $distfuncs{ $name };

	if ( $old && $new &&
	    $old->[0] eq $new->[0] &&
	    $old->[1] eq $new->[1] &&
	    $old->[2] eq $new->[2] ) {
		warn( "ok: $name\n" );
		next;
	}

	unless ( $old ) {
		warn( "$name: no old function\n" );
		next;
	}
	elsif ( ! $new ) {
		warn( "$name: no new function\n" );
		next;
	}
	elsif ( $old->[0] ne $new->[0] ) {

		# static?
		if ( $new->[0] =~ m!\A static \s+ (.+) !x && $1 eq $old->[0] ) {
			warn( "$name: local return value without static!\n" );
		}
		else {
			warn( "$name: different return value: old=<$old->[0]> != new=<$new->[0]>\n" );

			# more work to do...
			if ( $new->[0] eq 'Datum' ) {
				my $code = $new->[2];
				my @args;

				warn( "find args in $code" );

				my $reifdef = "[ \\t]* \\# [ \\t]* ifdef [ \\t]+ NOT_USED [ \\t]* [\\r\\n]+";
				my $regetarg = "( \\s+ ([^=;(){}#]+?) \\s* = \\s* PG_GETARG_([A-Z0-9_]+) \\( ([0-9]+) \\) ; ) [\\r\\n]+";
				my $reendif = "[ \\t]* \\# [ \\t]* endif [ \\t]* [\\r\\n]+";

				while ( $code =~ s! ^ ($reifdef) $regetarg $reendif !!msx ||
				    $code =~ s! ^ () $regetarg !!msx ) {
					my $unused = $1 ? 1 : 0;
					my $line = $2;
					my $var = $3;
					my $macro = $4;
					my $pos = $5;
					my $type = anonvar( $var );

					$var = cleanargs( $var );

					$typemap{ $macro } = $type;

					if ( $unused ) {
						warn( "NOT USED: ###$line###\n" );
						next;
					}

					warn( " - indirect argument: $var - ARG[$pos] - $macro" );

					push( @args, $var );

				}

				if ( $new->[1] eq 'PG_FUNCTION_ARGS' ) {
					$new->[1] = join( ', ', @args );
				}

				unless ( $code =~ s!
				    ^ (\s*) PG_RETURN_([A-Z0-9_]+)
				      \s* \(
				      \s* ( .*? )
				      \s* \) ( \s* ; \s* \} \s* )
				    \z!$1return $3$4!msx ) {
					warn( "no return value found!" );
					next;
				}

				my $ret = $2;
				my $var = $3;

				warn( "ret=<$ret>, var=<$var>" );

				# lookup return type
				unless ( $code =~ m!
				    ^ \s* ([A-Za-z0-9 \t]+	# type decl
				    (?: \s*\* | \s+ )		# delimiter
				    \Q$var\E \s* ) [;,]
				    !msx ) {
					warn( "!!! failed to find type: $var\n" );
				}

				my $retype = anonvar( $1 );


				if ( $retype ) {
					warn( "!!! retype: $retype\n" );

					$new->[0] = $retype;
				}

				$new->[2] = $code;

				warn( "updated code: <<<$new->[0]\n$name($new->[1])$code>>>" );
			}

			next;
		}

	}
	elsif ( $old->[1] ne $new->[1] ) {
		warn( "$name: different arguments: <$old->[1]> != <$new->[1]>\n" );
		next;
	}
	elsif ( $old->[2] ne $new->[2] ) {
		warn( "$name: different function body: <$old->[2]> != <$new->[2]>\n" );
		next;
	}

	warn( "$name: same\n" );
}

warn( Dumper( \%typemap ) );
