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

Datizo - Perl extension for the Datizo library to handle Date, Time and Timezones

=head1 SYNOPSIS

  use Datizo;
  ...

=head1 DESCRIPTION

XS binding for the Datizo library.

=head2 EXPORT

None by default.

=head1 SEE ALSO

DateTime on CPAN

L<https://github.com/sbertrang/datizo>

=head1 AUTHOR

Simon Bertrang, E<lt>janus@cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2012 by Simon Bertrang

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.12.2 or,
at your option, any later version of Perl 5 you may have available.

=cut

