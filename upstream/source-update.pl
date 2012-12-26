#!/usr/bin/perl

#
# scan for source comments, then check, compare and eventually update
#

use strict;
use warnings;
use Cwd qw( realpath );
use Data::Dumper qw( Dumper );
use File::Temp qw( tempfile );

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

	$string =~ s!\b int ( 8 | 16 | 32 | 64 ) \b!int${1}_t!gx;

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

# load local sources
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

	# skip files without postgresql source path comment
	unless ( $libsrcs{ $libsrc } =~ m! ^/[*][ ]postgresql:[ ](.+)[ ][*]/\n !x ) {
		warn( "no source comment found: $libsrc\n" );
		next;
	}

	# local source file to distfile mapping
	$libdists{ $libsrc } = $1;

	# reverse map from distfile to local sources
	push( @{ $distlibs{ $1 } }, $libsrc );
}

my @distsrcs = sort( keys( %distlibs ) );
my %distsrcs;


warn( Dumper( {
	'distlibs'	=> \%distlibs,
#	'libdists'	=> \%libdists,
	'distsrcs'	=> \@distsrcs,
} ) );

# load dist sources as indicated by source path comments
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
my $refuncargs = "\\( \\s* ([^;()]*?) \\s* \\)";
my $refuncbody = "( \\n \\{ \\n .*? \\n \\} (?: [ \\t]* /[*] [^\\n]*? [*]/ )? [ \\t]* \\n )";

my @libfuncs;
my %libfuncs;

# iterate local sources to find functions
for my $libsrc ( @libsrcs ) {
	# function name based on file
	my ( $func ) = $libsrc =~ m!\A (.+) [.] c \z!x;

	# keep the function names around
	push( @libfuncs, $func );

	# lookup function and body
	if ( $libsrcs{ $libsrc } =~ m! ^ ( $refuncret $func $refuncargs $refuncbody ) !msx ) {
		my $source = $1;
		my $ret = $2;
		my $args = $3;
		my $body = $4;

		warn( "found old function: $ret - $func ( $args )\n$body\n\n" );

		$libfuncs{ $func } = {
			'returns'	=> cleanargs( $ret ),
			'arguments'	=> cleanargs( $args ),
			'body'		=> $body,
			'source'	=> $source,
		};
	}
	else {
		warn( "could not find old function: $func\n" );
	}
}

my $relibfuncs = join( "|", map( quotemeta, @libfuncs ) );

my %distfuncs;

# iterate dist sources to find functions
for my $distsrc ( @distsrcs ) {
	# scan in a loop
	while ( $distsrcs{ $distsrc } =~ m! ^ $refuncret ($relibfuncs) $refuncargs $refuncbody !gmsx ) {
		my $ret = $1;
		my $func = $2;
		my $body = $4;
		my $args = $3;

		my $distfunc = $distfuncs{ $func } = {
			'returns'	=> cleanargs( $ret ),
			'arguments'	=> cleanargs( $args ),
			'body'		=> $body,
		};

		#warn( "found new function: $distfunc->{returns} - $func ( $distfunc->{arguments} )\n$body\n\n" );

	}
}

warn( "compare results...\n" );

# map postgresql macros to real types
my %typemap;
my @later;

for my $name ( @libfuncs ) {
	my $old = exists( $libfuncs{ $name } ) ? $libfuncs{ $name } : undef;
	my $new = exists( $distfuncs{ $name } ) ? $distfuncs{ $name } : undef;

	# adjust a few function names and types
	fixup_types( $new );

	# without an old version take the new one
	unless ( $old ) {
		warn( "no $name: using new function\n" );
	}

	# ignore without a new version
	unless ( $new ) {
		warn( "$name: no new function, bad!\n" );
		next;
	}

	# check return type
	if ( $old->{returns} ne $new->{returns} ) {

		# ignore the "static" prefix
		if ( $new->{returns} =~ m!\A static \s+ (.+) !x && $1 eq $old->{returns} ) {
			warn( "$name: remove static keyword from return value: $1\n" );
			# change to value without prefix
			$new->{returns} = $1;
		}

		if ( $new->{returns} eq 'Datum' ) {
			fixup_datum_func( $name, $new );
		}

	}

	if ( $old->{arguments} ne $new->{arguments} ) {
		warn( "$name: different arguments: <$old->{arguments}> != <$new->{arguments}>\n" );
		#next;
	}

	if ( $old->{body} ne $new->{body} ) {
		warn( "$name: different function body: <$old->{body}> != <$new->{body}>\n" );
		#next;
	}

	# when everything is the same, do nothing
	if ( $old && $new &&
	    $old->{returns} eq $new->{returns} &&
	    $old->{arguments} eq $new->{arguments} &&
	    $old->{body} eq $new->{body} ) {
		warn( "SAME: $name\n" );
		next;
	}

	warn( "DIFFERENT: $name\n" );

	# XXX: DO NOT DO IT
	#next;

	my $data = $libsrcs{ "$name.c" };

	$data =~ s! \Q$old->{source}\E !$new->{returns}\n$name($new->{arguments})$new->{body}!x;

	warn( "new file content:\n$data\n" );

	my ( $fh, $temp ) = tempfile( 'XXXXXXXXX',
		'DIR'	=> $libdir,
	);
	print( $fh $data );
	close( $fh );

	rename( $temp, "$libdir/$name.c" );
}

warn( Dumper( \%typemap ) );



sub fixup_datum_func
{
	my $name = shift;
	my $new = shift;

	my $body = $new->{body};
	my @args;

	#warn( "find args in $body" );

	my $reifdef = "[ \\t]* \\# [ \\t]* ifdef [ \\t]+ NOT_USED [ \\t]* [\\r\\n]+";
	my $regetarg = "( \\s+ ([^/=;(){}#]+?) \\s* = \\s* PG_GETARG_([A-Z0-9_]+) \\( ([0-9]+) \\) ; ) [\\r\\n]+";
	my $reendif = "[ \\t]* \\# [ \\t]* endif [ \\t]* [\\r\\n]+";

	while ( $body =~ s! ^ ($reifdef) $regetarg $reendif !!msx ||
	    $body =~ s! ^ () $regetarg !!msx ) {
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

		#warn( " - indirect argument: $var - ARG[$pos] - $macro" );

		push( @args, $var );

	}

	if ( $new->{arguments} eq 'PG_FUNCTION_ARGS' ) {
		$new->{arguments} = join( ', ', @args );
	}

	unless ( $body =~ s!
	    ^ (\s*) PG_RETURN_([A-Z0-9_]+)
	      \s* \(
	      \s* ( [a-zA-Z0-9_]*? )
	      \s* \) ( \s* ; \s* \} \s* )
	    \z!$1return $3$4!msx ) {
		warn( "no return value found!" );
		next;
	}

	my $ret = $2;
	my $var = $3;

	#warn( "ret=<$ret>, var=<$var>" );

	# lookup return type
	unless ( $body =~ m!
	    ^ \s* ([A-Za-z0-9 \t]+	# type decl
	    (?: \s*\* | \s+ )		# delimiter
	    \Q$var\E \s* ) [;,]
	    !msx ) {
		warn( "!!! failed to find type: $var\n" );
	}

	my $retype = anonvar( $1 );

	$retype =~ s!(struct\s+pg_tm)!${1}tm!g;
	$body =~ s!(struct\s+)pg_tm!${1}tm!g;

	if ( $retype ) {
		#warn( "!!! retype: $retype\n" );

		$new->{returns} = $retype;
	}

	$new->{body} = $body;

	#warn( "updated code: <<<$new->{returns}\n$name($new->{arguments})$body>>>" );
}

sub fixup_types
{
	my $new = shift;

	# do nothing without code
	unless ( exists( $new->{body}) && defined( $new->{body} ) ) {
		return;
	}

	$new->{body} =~ s!(struct\s+)pg_tm!${1}tm!gx;
	$new->{body} =~ s!\b int (8|16|32|64) \b!int${1}_t!gx;
	$new->{body} =~ s!\b Assert \( !assert(!gx;
	$new->{body} =~ s!\b pstrdup \( !strdup(!gx;
	$new->{body} =~ s!\b palloc \( !malloc(!gx;
	$new->{body} =~ s!\b MAXPGPATH \b!MAXPATHLEN!gx;

	# return open(fullname, O_RDONLY | PG_BINARY, 0);
	$new->{body} =~ s! [ ][|][ ] PG_BINARY [,] !,!gx;

	# two cases of uncatched returns
	$new->{body} =~ s! PG_RETURN_DATEADT \s* \( \s* date \s* \) \s* ; !return date;!gmsx;

=ereport

ereport( ERROR, (
    errcode( ERRCODE_DATETIME_FIELD_OVERFLOW ),
    errmsg( "date/time field value out of range: \"%s\"", str )
) );

ereport( ERROR, (
    errcode( ERRCODE_INVALID_DATETIME_FORMAT ),
    errmsg( "invalid input syntax for type %s: \"%s\"", datatype, str )
) );

ereport( ERROR, (
    errcode( ERRCODE_INVALID_PARAMETER_VALUE ),
    errmsg( "timestamp(%d) precision must be between %d and %d", typmod, 0, MAX_TIMESTAMP_PRECISION )
) );

elog( ERROR, "unrecognized interval typmod: %d", typmod );

errhint("Perhaps you need a different \"datestyle\" setting.")));



=cut

	$new->{body} =~ s!
	    (\s+) ereport \s* \( \s* ERROR \s* , \s* \( \s*
	      errcode \s* \( \s* [A-Za-z0-9_]+ \s* \) \s* , \s*
	      errmsg \s* ( \( \s* .+? \s* \) ) \s*
	      (?: , \s* errhint \s* \( \s* .+? \s* \) \s* )?
	    \) \s* \) \s* ; !${1}warnx${2};!gmsx;

	$new->{body} =~ s!
	    (\s+) elog \s* \( \s* ERROR \s* , \s* (.+?) \s* \) \s* ;
	    !${1}warnx(${2});!gmsx;
}

