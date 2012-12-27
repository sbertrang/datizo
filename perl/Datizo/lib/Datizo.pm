package Datizo;

use strict;
use warnings;

our $VERSION = '0.01';

require XSLoader;

XSLoader::load('Datizo', $VERSION);

1;

package Datizo::TimestampTz;

use strict;
use warnings;
use overload
    'cmp' => \&Datizo::TimestampTz::cmp,
    '<=>' => \&Datizo::TimestampTz::cmp,
    '""' => \&Datizo::TimestampTz::to_string
;

1;

package Datizo::Timezone;

use strict;
use warnings;
use overload
    '""' => \&Datizo::Timezone::to_string
;

1;

package Datizo::Interval;

use strict;
use warnings;
use overload
    '""' => \&Datizo::Interval::to_string
;

1;

__END__

=head1 NAME

Datizo - Perl extension for blah blah blah

=head1 SYNOPSIS

  use Datizo;
  blah blah blah

=head1 DESCRIPTION

Stub documentation for Datizo, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.

Blah blah blah.

=head2 EXPORT

None by default.



=head1 SEE ALSO

Mention other useful documentation such as the documentation of
related modules or operating system documentation (such as man pages
in UNIX), or any relevant external documentation such as RFCs or
standards.

If you have a mailing list set up for your module, mention it here.

If you have a web site set up for your module, mention it here.

=head1 AUTHOR

A. U. Thor, E<lt>simon@E<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2012 by A. U. Thor

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.12.2 or,
at your option, any later version of Perl 5 you may have available.


=cut
