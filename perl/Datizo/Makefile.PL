
use strict;
use warnings;

use ExtUtils::MakeMaker;

WriteMakefile(
    NAME              => 'Datizo',
    VERSION_FROM      => 'lib/Datizo.pm',
    PREREQ_PM         => {},
    ABSTRACT_FROM     => 'lib/Datizo.pm',
    AUTHOR            => 'Simon Bertrang <janus@cpan.org>',
    LIBS              => ['-L../../lib -ldatizo'],
    DEFINE            => '',
    INC               => '-I../../lib',
    # OBJECT            => '$(O_FILES)',
    OPTIMIZE          => '-g',
);

