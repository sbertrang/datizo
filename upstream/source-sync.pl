#!/usr/bin/perl

use strict;
use warnings;

use Cwd qw( realpath );
use Data::Dumper qw( Dumper );
use File::Find qw( find );
use File::Temp qw( tempfile );

my $libdir = realpath( "../lib" );
my $distdir = realpath( "./postgresql-9.2.2" );
my ( $distname ) = $distdir =~ m! \b ([^/]+) \z!x;

#
# lists of library sources and distribution source files
#

my @libsrcs = map m!\A $libdir / (.+) \z!x ? $1 : (), glob( "$libdir/*.c" );
my @distsrcs;

find({
	no_chdir => 1,
	wanted => sub {
		# only source code
		return unless m! [.] c \z!x;

		# ignore some files
		return if m! /interfaces/ecpg/ !x;

		# keep relative path
		push( @distsrcs, substr( $_, length( $distdir ) + 1 ) );
	},
}, $distdir );

printf( STDOUT "libdir: %s (%d)\ndistdir: %s (%d)\n",
	$libdir, scalar( @libsrcs ),
	$distdir, scalar( @distsrcs ),
);

#warn( Dumper( { libsrcs => \@libsrcs } ) );

#
# load library sources
#

my %libsrcs;

for my $src ( @libsrcs ) {
	if ( open( my $fh, '<', "$libdir/$src" ) ) {
		local( $/ );
		$libsrcs{ $src } = <$fh>;
		close( $fh );
	}
	else {
		warn( "$!: $libdir/$src" );
		next;
	}
}


#
# ensure file matches function
#

my %libargs;

for my $src ( @libsrcs ) {
	my ( $name ) = $src =~ m! (.+) [.] c \z!x;

	unless ( $libsrcs{ $src } =~ m! ^ $name \s* \( \s* (.*?) \s* \) \s* \{ !msx ) {
		delete( $libsrcs{ $src } );
		warn( "function $name not found in $src!\n" );
		next;
	}

	$libargs{ $src } = cleanargs( $1 );
}

#
# remove sources where the file name does not match a function
#

@libsrcs = sort( grep( exists( $libsrcs{ $_ } ), @libsrcs ) );

#
# map from function name to file
#

my %libnames = map( m!(.+)[.]c\z!x ? ($1,$_) : (), @libsrcs );
my @libnames = sort( keys( %libnames ) );

#
# build regular expression to scan disttribution sources
#

my $libnames = join( '|', map( quotemeta, @libnames ) );

# function to distfile
my %distlibs;
my %distsrcs;

#
# scan dist sources
#

for my $distsrc ( @distsrcs ) {
	my $data;

	if ( open( my $fh, '<', "$distdir/$distsrc" ) ) {
		local( $/ );
		$data = <$fh>;
		close( $fh );
	}
	else {
		warn( "$!: $distdir/$distsrc" );
		next;
	}

	warn( "scanning $distsrc ...\n" );

	while ( $data =~ m! ^ ($libnames) \s* \( \s* (.*?) \s* \) \s* \{ !gmsx ) {
		my $name = $1;
		my $args = cleanargs( $2 );
		my $libsrc = $libnames{ $name };

		warn( "found function $name ( $args )\n" );

		unless ( $args eq $libargs{ $libsrc } ) {
			warn( "arguments do not match: <$args> != <$libargs{$libsrc}>" );
			next;
		}

		push( @{ $distlibs{ $name } }, $distsrc );

		$distsrcs{ $distsrc } ||= $data;
	}
}


# show which functions refer to which dist sources

for my $name ( sort( keys( %distlibs ) ) ) {
	warn( "function $name:\n" );

	my $distsrcs = $distlibs{ $name };

	if ( @$distsrcs == 0 ) {
		warn( "no sources found!\n" );
		next;
	}
	elsif ( @$distsrcs > 1 ) {
		warn( "found too many?!\n" );
		for my $distsrc ( @{ $distlibs{ $name } } ) {
			warn( " - $distsrc\n" );
		}
		next;
	}

	my $distsrc = $distsrcs->[0];
	my $libsrc = $libnames{ $name };

	if ( $libsrcs{ $libsrc } =~ m! ^ ( /\* [ ] postgresql: [ ] .+? [ ] \*/ ) $ !x ) {
		warn( "found comment: $1\n" );
	}
	else {
		my $comment = "/* postgresql: $distsrc */\n";

		warn( "add comment: $comment\n" );

		my ( $fh, $temp ) = tempfile( "XXXXXXXXX",
			'DIR'	=> $libdir,
		);
		print( $fh $comment );
		print( $fh $libsrcs{ $libsrc } );
		close( $fh );

		rename( $temp, "$libdir/$libsrc" );
	}
}



#
# helper functions
#

sub cleanargs
{
	my $string = shift;

	$string =~ s! \s+ ! !gmsx;
	#$string =~ s! \* [ ] [a-zA-Z0-9_]+ ! !gx;

	return $string;
}
