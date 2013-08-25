# Datizo - Library for Dates, Times and Timezones

This is an attempt to isolate the datetime functionality from PostgreSQL and
make it available to other systems and languages too.

So far the strategy has been to automate this as much as possible to be able to
easily sync with upstream (see upstream/source-update.pl).

## State

EXPERIMENTAL: Initial functionality is working, but some parts are still missing.
Namely the formating functions and additional timezones need to be loaded.

## Future

In the long run maybe this could lead to a joint effort of multiple projects.

# Todo

- change default datestyle to YMD (should be fine outside SQL context)
- dual-life makefiles, shared config via includes, specific implementation for
  GNU and BSD make
- implement loading of remaining timezones

# Notes

http://www.postgresql.org/docs/current/static/datatype-datetime.html

## Table 8-14. Date/Time Output Styles

Specification  Description             Example
ISO            ISO 8601, SQL standard  1997-12-17 07:37:16-08
SQL            traditional style       12/17/1997 07:37:16.00 PST
Postgres       original style          Wed Dec 17 07:37:16 1997 PST
German         regional style          17.12.1997 07:37:16.00 PST

http://www.postgresql.org/docs/current/static/runtime-config-client.html#GUC-DATESTYLE

## DateStyle (string)
Sets the display format for date and time values, as well as the rules for
interpreting ambiguous date input values. For historical reasons, this variable
contains two independent components: the output format specification (ISO,
Postgres, SQL, or German) and the input/output specification for year/month/day
ordering (DMY, MDY, or YMD). These can be set separately or together. The
keywords Euro and European are synonyms for DMY; the keywords US, NonEuro, and
NonEuropean are synonyms for MDY. See Section 8.5 for more information. The
built-in default is ISO, MDY, but initdb will initialize the configuration file
with a setting that corresponds to the behavior of the chosen lc_time locale.


## IntervalStyle (enum)
Sets the display format for interval values. The value sql_standard will produce
output matching SQL standard interval literals. The value postgres (which is the
default) will produce output matching PostgreSQL releases prior to 8.4 when the
DateStyle parameter was set to ISO. The value postgres_verbose will produce
output matching PostgreSQL releases prior to 8.4 when the DateStyle parameter
was set to non-ISO output. The value iso_8601 will produce output matching the
time interval "format with designators" defined in section 4.4.3.2 of ISO 8601.

The IntervalStyle parameter also affects the interpretation of ambiguous
interval input. See Section 8.5.4 for more information.


## currently missing is loading the tznames/

this happens via

  src/backend/utils/misc/tzparser.c:load_tzoffsets()

which is called from

  src/backend/utils/misc/guc.c:check_timezone_abbreviations()


the other probably relevant call in this file is called from

  src/backend/utils/misc/guc.c:assign_timezone_abbreviations()

which comes from

  src/backend/utils/adt/datetime.c:InstallTimeZoneAbbrevs()


### entry point to the GUC things:

  src/backend/utils/misc/guc.c:InitializeGUCOptions()

this in fact calls pg_timezone_initialize()

then build_guc_variables()

and afterwards initializes all GUC options with InitializeOneGUCOption()

## Testing Perl on OSX

Needs "export DYLD_LIBRARY_PATH=...datizo/lib" before "make test"


# License

PostgreSQL license (BSD-like): http://opensource.org/licenses/postgresql
