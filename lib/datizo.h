#ifndef	__DATIZO_H__
#define	__DATIZO_H__

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <time.h>

/*******************************************************************************
********************************************************************************
**  src/include/c.h  ***********************************************************
********************************************************************************
*******************************************************************************/

#define INT64CONST(x)  ((int64_t) x)
#define UINT64CONST(x) ((uint64_t) x)


/*******************************************************************************
********************************************************************************
**  src/include/datatype/timestamp.h  ******************************************
********************************************************************************
*******************************************************************************/

typedef	int64_t	Timestamp;
typedef	int64_t	TimestampTz;
typedef int64_t	TimeOffset;
typedef int32_t	fsec_t;			/* fractional seconds (in microseconds) */

typedef struct
{
	TimeOffset	time;			/* all time units other than days, months and
								 * years */
	int32_t		day;			/* days, after time for alignment */
	int32_t		month;			/* months and years, after time for alignment */
} Interval;


#define MAX_TIMESTAMP_PRECISION 6
#define MAX_INTERVAL_PRECISION 6

/*
 *	Round off to MAX_TIMESTAMP_PRECISION decimal places.
 *	Note: this is also used for rounding off intervals.
 */
#define TS_PREC_INV 1000000.0
#define TSROUND(j) (rint(((double) (j)) * TS_PREC_INV) / TS_PREC_INV)


/*
 * Assorted constants for datetime-related calculations
 */

#define DAYS_PER_YEAR	365.25	/* assumes leap year every four years */
#define MONTHS_PER_YEAR 12
/*
 *	DAYS_PER_MONTH is very imprecise.  The more accurate value is
 *	365.2425/12 = 30.436875, or '30 days 10:29:06'.  Right now we only
 *	return an integral number of days, but someday perhaps we should
 *	also return a 'time' value to be used as well.	ISO 8601 suggests
 *	30 days.
 */
#define DAYS_PER_MONTH	30		/* assumes exactly 30 days per month */
#define HOURS_PER_DAY	24		/* assume no daylight savings time changes */

/*
 *	This doesn't adjust for uneven daylight savings time intervals or leap
 *	seconds, and it crudely estimates leap years.  A more accurate value
 *	for days per years is 365.2422.
 */
#define SECS_PER_YEAR	(36525 * 864)	/* avoid floating-point computation */
#define SECS_PER_DAY	86400
#define SECS_PER_HOUR	3600
#define SECS_PER_MINUTE 60
#define MINS_PER_HOUR	60

#define USECS_PER_DAY	INT64CONST(86400000000)
#define USECS_PER_HOUR	INT64CONST(3600000000)
#define USECS_PER_MINUTE INT64CONST(60000000)
#define USECS_PER_SEC	INT64CONST(1000000)

/*
 * We allow numeric timezone offsets up to 15:59:59 either way from Greenwich.
 * Currently, the record holders for wackiest offsets in actual use are zones
 * Asia/Manila, at -15:56:00 until 1844, and America/Metlakatla, at +15:13:42
 * until 1867.  If we were to reject such values we would fail to dump and
 * restore old timestamptz values with these zone settings.
 */
#define MAX_TZDISP_HOUR		15				/* maximum allowed hour part */
#define TZDISP_LIMIT		((MAX_TZDISP_HOUR + 1) * SECS_PER_HOUR)



#define DT_NOBEGIN		(-INT64CONST(0x7fffffffffffffff) - 1)
#define DT_NOEND		(INT64CONST(0x7fffffffffffffff))

#define TIMESTAMP_NOBEGIN(j)	\
	do {(j) = DT_NOBEGIN;} while (0)

#define TIMESTAMP_IS_NOBEGIN(j) ((j) == DT_NOBEGIN)

#define TIMESTAMP_NOEND(j)		\
	do {(j) = DT_NOEND;} while (0)

#define TIMESTAMP_IS_NOEND(j)	((j) == DT_NOEND)

#define TIMESTAMP_NOT_FINITE(j) (TIMESTAMP_IS_NOBEGIN(j) || TIMESTAMP_IS_NOEND(j))


/*
 * Julian date support.
 *
 * IS_VALID_JULIAN checks the minimum date exactly, but is a bit sloppy
 * about the maximum, since it's far enough out to not be especially
 * interesting.
 */

#define JULIAN_MINYEAR (-4713)
#define JULIAN_MINMONTH (11)
#define JULIAN_MINDAY (24)
#define JULIAN_MAXYEAR (5874898)

#define IS_VALID_JULIAN(y,m,d) \
	(((y) > JULIAN_MINYEAR \
	  || ((y) == JULIAN_MINYEAR && \
		  ((m) > JULIAN_MINMONTH \
		   || ((m) == JULIAN_MINMONTH && (d) >= JULIAN_MINDAY)))) \
	 && (y) < JULIAN_MAXYEAR)

#define JULIAN_MAX (2147483494)			/* == date2j(JULIAN_MAXYEAR, 1, 1) */

/* Julian-date equivalents of Day 0 in Unix and Postgres reckoning */
#define UNIX_EPOCH_JDATE		2440588 /* == date2j(1970, 1, 1) */
#define POSTGRES_EPOCH_JDATE	2451545 /* == date2j(2000, 1, 1) */

/*******************************************************************************
********************************************************************************
**  src/include/utils/timestamp.h  *********************************************
********************************************************************************
*******************************************************************************/


#define TIMESTAMP_MASK(b) (1 << (b))
#define INTERVAL_MASK(b) (1 << (b))

/* Macros to handle packing and unpacking the typmod field for intervals */
#define INTERVAL_FULL_RANGE (0x7FFF)
#define INTERVAL_RANGE_MASK (0x7FFF)
#define INTERVAL_FULL_PRECISION (0xFFFF)
#define INTERVAL_PRECISION_MASK (0xFFFF)
#define INTERVAL_TYPMOD(p,r) ((((r) & INTERVAL_RANGE_MASK) << 16) | ((p) & INTERVAL_PRECISION_MASK))
#define INTERVAL_PRECISION(t) ((t) & INTERVAL_PRECISION_MASK)
#define INTERVAL_RANGE(t) (((t) >> 16) & INTERVAL_RANGE_MASK)

#ifdef HAVE_INT64_TIMESTAMP
#define TimestampTzPlusMilliseconds(tz,ms) ((tz) + ((ms) * (int64_t) 1000))
#else
#define TimestampTzPlusMilliseconds(tz,ms) ((tz) + ((ms) / 1000.0))
#endif





/*******************************************************************************
********************************************************************************
**  src/include/utils/datetime.h  **********************************************
********************************************************************************
*******************************************************************************/

/* ----------------------------------------------------------------
 *				time types + support macros
 *
 * String definitions for standard time quantities.
 *
 * These strings are the defaults used to form output time strings.
 * Other alternative forms are hardcoded into token tables in datetime.c.
 * ----------------------------------------------------------------
 */

#define DAGO			"ago"
#define DCURRENT		"current"
#define EPOCH			"epoch"
#define INVALID			"invalid"
#define EARLY			"-infinity"
#define LATE			"infinity"
#define NOW				"now"
#define TODAY			"today"
#define TOMORROW		"tomorrow"
#define YESTERDAY		"yesterday"
#define ZULU			"zulu"

#define DMICROSEC		"usecond"
#define DMILLISEC		"msecond"
#define DSECOND			"second"
#define DMINUTE			"minute"
#define DHOUR			"hour"
#define DDAY			"day"
#define DWEEK			"week"
#define DMONTH			"month"
#define DQUARTER		"quarter"
#define DYEAR			"year"
#define DDECADE			"decade"
#define DCENTURY		"century"
#define DMILLENNIUM		"millennium"
#define DA_D			"ad"
#define DB_C			"bc"
#define DTIMEZONE		"timezone"

/*
 * Fundamental time field definitions for parsing.
 *
 *	Meridian:  am, pm, or 24-hour style.
 *	Millennium: ad, bc
 */

#define AM		0
#define PM		1
#define HR24	2

#define AD		0
#define BC		1

/*
 * Fields for time decoding.
 *
 * Can't have more of these than there are bits in an unsigned int
 * since these are turned into bit masks during parsing and decoding.
 *
 * Furthermore, the values for YEAR, MONTH, DAY, HOUR, MINUTE, SECOND
 * must be in the range 0..14 so that the associated bitmasks can fit
 * into the left half of an INTERVAL's typmod value.  Since those bits
 * are stored in typmods, you can't change them without initdb!
 */

#define RESERV	0
#define MONTH	1
#define YEAR	2
#define DAY		3
#define JULIAN	4
#define TZ		5
#define DTZ		6
#define DTZMOD	7
#define IGNORE_DTF	8
#define AMPM	9
#define HOUR	10
#define MINUTE	11
#define SECOND	12
#define MILLISECOND 13
#define MICROSECOND 14
#define DOY		15
#define DOW		16
#define UNITS	17
#define ADBC	18
/* these are only for relative dates */
#define AGO		19
#define ABS_BEFORE		20
#define ABS_AFTER		21
/* generic fields to help with parsing */
#define ISODATE 22
#define ISOTIME 23
/* these are only for parsing intervals */
#define WEEK		24
#define DECADE		25
#define CENTURY		26
#define MILLENNIUM	27
/* reserved for unrecognized string values */
#define UNKNOWN_FIELD	31

/*
 * Token field definitions for time parsing and decoding.
 * These need to fit into the datetkn table type.
 * At the moment, that means keep them within [-127,127].
 * These are also used for bit masks in DecodeDateDelta()
 *	so actually restrict them to within [0,31] for now.
 * - thomas 97/06/19
 * Not all of these fields are used for masks in DecodeDateDelta
 *	so allow some larger than 31. - thomas 1997-11-17
 */

#define DTK_NUMBER		0
#define DTK_STRING		1

#define DTK_DATE		2
#define DTK_TIME		3
#define DTK_TZ			4
#define DTK_AGO			5

#define DTK_SPECIAL		6
#define DTK_INVALID		7
#define DTK_CURRENT		8
#define DTK_EARLY		9
#define DTK_LATE		10
#define DTK_EPOCH		11
#define DTK_NOW			12
#define DTK_YESTERDAY	13
#define DTK_TODAY		14
#define DTK_TOMORROW	15
#define DTK_ZULU		16

#define DTK_DELTA		17
#define DTK_SECOND		18
#define DTK_MINUTE		19
#define DTK_HOUR		20
#define DTK_DAY			21
#define DTK_WEEK		22
#define DTK_MONTH		23
#define DTK_QUARTER		24
#define DTK_YEAR		25
#define DTK_DECADE		26
#define DTK_CENTURY		27
#define DTK_MILLENNIUM	28
#define DTK_MILLISEC	29
#define DTK_MICROSEC	30
#define DTK_JULIAN		31

#define DTK_DOW			32
#define DTK_DOY			33
#define DTK_TZ_HOUR		34
#define DTK_TZ_MINUTE	35
#define DTK_ISOYEAR		36
#define DTK_ISODOW		37


/*
 * Bit mask definitions for time parsing.
 */

#define DTK_M(t)		(0x01 << (t))

/* Convenience: a second, plus any fractional component */
#define DTK_ALL_SECS_M	(DTK_M(SECOND) | DTK_M(MILLISECOND) | DTK_M(MICROSECOND))
#define DTK_DATE_M		(DTK_M(YEAR) | DTK_M(MONTH) | DTK_M(DAY))
#define DTK_TIME_M		(DTK_M(HOUR) | DTK_M(MINUTE) | DTK_ALL_SECS_M)

#define MAXDATELEN		63		/* maximum possible length of an input date
								 * string (not counting tr. null) */
#define MAXDATEFIELDS	25		/* maximum possible number of fields in a date
								 * string */
#define TOKMAXLEN		10		/* only this many chars are stored in
								 * datetktbl */

/* keep this struct small; it gets used a lot */
typedef struct
{
	char		token[TOKMAXLEN];
	char		type;
	char		value;			/* this may be unsigned, alas */
} datetkn;

/* one of its uses is in tables of time zone abbreviations */
typedef struct TimeZoneAbbrevTable
{
	int			numabbrevs;
	datetkn		abbrevs[1];		/* VARIABLE LENGTH ARRAY */
} TimeZoneAbbrevTable;


/* FMODULO()
 * Macro to replace modf(), which is broken on some platforms.
 * t = input and remainder
 * q = integer part
 * u = divisor
 */
#define FMODULO(t,q,u) \
do { \
	(q) = (((t) < 0) ? ceil((t) / (u)) : floor((t) / (u))); \
	if ((q) != 0) (t) -= rint((q) * (u)); \
} while(0)

/* TMODULO()
 * Like FMODULO(), but work on the timestamp datatype (either int64 or float8).
 * We assume that int64 follows the C99 semantics for division (negative
 * quotients truncate towards zero).
 */
#ifdef HAVE_INT64_TIMESTAMP
#define TMODULO(t,q,u) \
do { \
	(q) = ((t) / (u)); \
	if ((q) != 0) (t) -= ((q) * (u)); \
} while(0)
#else
#define TMODULO(t,q,u) \
do { \
	(q) = (((t) < 0) ? ceil((t) / (u)) : floor((t) / (u))); \
	if ((q) != 0) (t) -= rint((q) * (u)); \
} while(0)
#endif

/*
 * Date/time validation
 * Include check for leap year.
 */

extern const int day_tab[2][13];

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))



extern char	   *months[];
extern char	   *days[];

/*
 * Definitions for squeezing values into "value"
 * We set aside a high bit for a sign, and scale the timezone offsets
 * in minutes by a factor of 15 (so can represent quarter-hour increments).
 */
#define ABS_SIGNBIT		((char) 0200)
#define VALMASK			((char) 0177)
#define POS(n)			(n)
#define NEG(n)			((n)|ABS_SIGNBIT)
#define SIGNEDCHAR(c)	((c)&ABS_SIGNBIT? -((c)&VALMASK): (c))
#define FROMVAL(tp)		(-SIGNEDCHAR((tp)->value) * 15) /* uncompress */
#define TOVAL(tp, v)	((tp)->value = ((v) < 0? NEG((-(v))/15): POS(v)/15))







/*
 * Datetime input parsing routines (ParseDateTime, DecodeDateTime, etc)
 * return zero or a positive value on success.	On failure, they return
 * one of these negative code values.  DateTimeParseError may be used to
 * produce a correct ereport.
 */
#define DTERR_BAD_FORMAT		(-1)
#define DTERR_FIELD_OVERFLOW	(-2)
#define DTERR_MD_FIELD_OVERFLOW (-3)	/* triggers hint about DateStyle */
#define DTERR_INTERVAL_OVERFLOW (-4)
#define DTERR_TZDISP_OVERFLOW	(-5)


/*******************************************************************************
********************************************************************************
**  src/timezone/tzfile.h  *****************************************************
********************************************************************************
*******************************************************************************/

/*
 * Information about time zone files.
 */

#define TZDEFAULT	"localtime"
#define TZDEFRULES	"posixrules"

/*
 * Each file begins with. . .
 */

#define TZ_MAGIC	"TZif"

struct tzhead
{
	char		tzh_magic[4];	/* TZ_MAGIC */
	char		tzh_version[1]; /* '\0' or '2' as of 2005 */
	char		tzh_reserved[15];		/* reserved--must be zero */
	char		tzh_ttisgmtcnt[4];		/* coded number of trans. time flags */
	char		tzh_ttisstdcnt[4];		/* coded number of trans. time flags */
	char		tzh_leapcnt[4]; /* coded number of leap seconds */
	char		tzh_timecnt[4]; /* coded number of transition times */
	char		tzh_typecnt[4]; /* coded number of local time types */
	char		tzh_charcnt[4]; /* coded number of abbr. chars */
};

/*----------
 * . . .followed by. . .
 *
 *	tzh_timecnt (char [4])s		coded transition times a la time(2)
 *	tzh_timecnt (unsigned char)s	types of local time starting at above
 *	tzh_typecnt repetitions of
 *		one (char [4])		coded UTC offset in seconds
 *		one (unsigned char) used to set tm_isdst
 *		one (unsigned char) that's an abbreviation list index
 *	tzh_charcnt (char)s		'\0'-terminated zone abbreviations
 *	tzh_leapcnt repetitions of
 *		one (char [4])		coded leap second transition times
 *		one (char [4])		total correction after above
 *	tzh_ttisstdcnt (char)s		indexed by type; if TRUE, transition
 *					time is standard time, if FALSE,
 *					transition time is wall clock time
 *					if absent, transition times are
 *					assumed to be wall clock time
 *	tzh_ttisgmtcnt (char)s		indexed by type; if TRUE, transition
 *					time is UTC, if FALSE,
 *					transition time is local time
 *					if absent, transition times are
 *					assumed to be local time
 *----------
 */

/*
 * If tzh_version is '2' or greater, the above is followed by a second instance
 * of tzhead and a second instance of the data in which each coded transition
 * time uses 8 rather than 4 chars,
 * then a POSIX-TZ-environment-variable-style string for use in handling
 * instants after the last transition time stored in the file
 * (with nothing between the newlines if there is no POSIX representation for
 * such instants).
 */

/*
 * In the current implementation, "tzset()" refuses to deal with files that
 * exceed any of the limits below.
 */

#define TZ_MAX_TIMES	1200

#define TZ_MAX_TYPES	256		/* Limited by what (unsigned char)'s can hold */

#define TZ_MAX_CHARS	50		/* Maximum number of abbreviation characters */
 /* (limited by what unsigned chars can hold) */

#define TZ_MAX_LEAPS	50		/* Maximum number of leap second corrections */

#define SECSPERMIN	60
#define MINSPERHOUR 60
#define HOURSPERDAY 24
#define DAYSPERWEEK 7
#define DAYSPERNYEAR	365
#define DAYSPERLYEAR	366
#define SECSPERHOUR (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	((long) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR 12

#define TM_SUNDAY	0
#define TM_MONDAY	1
#define TM_TUESDAY	2
#define TM_WEDNESDAY	3
#define TM_THURSDAY 4
#define TM_FRIDAY	5
#define TM_SATURDAY 6

#define TM_JANUARY	0
#define TM_FEBRUARY 1
#define TM_MARCH	2
#define TM_APRIL	3
#define TM_MAY		4
#define TM_JUNE		5
#define TM_JULY		6
#define TM_AUGUST	7
#define TM_SEPTEMBER	8
#define TM_OCTOBER	9
#define TM_NOVEMBER 10
#define TM_DECEMBER 11

#define TM_YEAR_BASE	1900

#define EPOCH_YEAR	1970
#define EPOCH_WDAY	TM_THURSDAY

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

/*
 * Since everything in isleap is modulo 400 (or a factor of 400), we know that
 *	  isleap(y) == isleap(y % 400)
 * and so
 *	  isleap(a + b) == isleap((a + b) % 400)
 * or
 *	  isleap(a + b) == isleap(a % 400 + b % 400)
 * This is true even if % means modulo rather than Fortran remainder
 * (which is allowed by C89 but not C99).
 * We use this to avoid addition overflow problems.
 */

#define isleap_sum(a, b)	  isleap((a) % 400 + (b) % 400)


/*******************************************************************************
********************************************************************************
**  src/include/pgtime.h  ******************************************************
********************************************************************************
*******************************************************************************/

typedef int64_t pg_time_t;

/* Maximum length of a timezone name (not including trailing null) */
#define TZ_STRLEN_MAX 255






/*******************************************************************************
********************************************************************************
**  src/timezone/pgtz.h  *******************************************************
********************************************************************************
*******************************************************************************/

#define BIGGEST(a, b)	(((a) > (b)) ? (a) : (b))

struct ttinfo
{								/* time type information */
	long		tt_gmtoff;		/* UTC offset in seconds */
	int			tt_isdst;		/* used to set tm_isdst */
	int			tt_abbrind;		/* abbreviation list index */
	int			tt_ttisstd;		/* TRUE if transition is std time */
	int			tt_ttisgmt;		/* TRUE if transition is UTC */
};

struct lsinfo
{								/* leap second information */
	pg_time_t	ls_trans;		/* transition time */
	long		ls_corr;		/* correction to apply */
};

struct state
{
	int			leapcnt;
	int			timecnt;
	int			typecnt;
	int			charcnt;
	int			goback;
	int			goahead;
	pg_time_t	ats[TZ_MAX_TIMES];
	unsigned char types[TZ_MAX_TIMES];
	struct ttinfo ttis[TZ_MAX_TYPES];
	char		chars[BIGGEST(BIGGEST(TZ_MAX_CHARS + 1, 3 /* sizeof gmt */ ),
										  (2 * (TZ_STRLEN_MAX + 1)))];
	struct lsinfo lsis[TZ_MAX_LEAPS];
};


struct pg_tz
{
	/* TZname contains the canonically-cased name of the timezone */
	char		TZname[TZ_STRLEN_MAX + 1];
	struct state state;
};

typedef struct pg_tz pg_tz;
typedef struct pg_tzenum pg_tzenum;






/*******************************************************************************
********************************************************************************
**  src/timezone/private.h  ****************************************************
********************************************************************************
*******************************************************************************/




#ifndef TYPE_BIT
#define TYPE_BIT(type)	(sizeof (type) * CHAR_BIT)
#endif   /* !defined TYPE_BIT */

#ifndef TYPE_SIGNED
#define TYPE_SIGNED(type) (((type) -1) < 0)
#endif   /* !defined TYPE_SIGNED */

/*
 * Since the definition of TYPE_INTEGRAL contains floating point numbers,
 * it cannot be used in preprocessor directives.
 */

#ifndef TYPE_INTEGRAL
#define TYPE_INTEGRAL(type) (((type) 0.5) != 0.5)
#endif   /* !defined TYPE_INTEGRAL */

#ifndef INT_STRLEN_MAXIMUM
/*
 * 302 / 1000 is log10(2.0) rounded up.
 * Subtract one for the sign bit if the type is signed;
 * add one for integer division truncation;
 * add one more for a minus sign if the type is signed.
 */
#define INT_STRLEN_MAXIMUM(type) \
	((TYPE_BIT(type) - TYPE_SIGNED(type)) * 302 / 1000 + 1 + TYPE_SIGNED(type))
#endif   /* !defined INT_STRLEN_MAXIMUM */



#ifndef YEARSPERREPEAT
#define YEARSPERREPEAT			400		/* years before a Gregorian repeat */
#endif   /* !defined YEARSPERREPEAT */

/*
** The Gregorian year averages 365.2425 days, which is 31556952 seconds.
*/

#ifndef AVGSECSPERYEAR
#define AVGSECSPERYEAR			31556952L
#endif   /* !defined AVGSECSPERYEAR */

#ifndef SECSPERREPEAT
#define SECSPERREPEAT			((int64_t) YEARSPERREPEAT * (int64_t) AVGSECSPERYEAR)
#endif   /* !defined SECSPERREPEAT */

#ifndef SECSPERREPEAT_BITS
#define SECSPERREPEAT_BITS		34		/* ceil(log2(SECSPERREPEAT)) */
#endif   /* !defined SECSPERREPEAT_BITS */






extern pg_tz          *session_timezone;
extern pg_tz          *log_timezone;



/*******************************************************************************
********************************************************************************
**  src/timezone/localtime.c  **************************************************
********************************************************************************
*******************************************************************************/

#ifndef WILDABBR
/*----------
 * Someone might make incorrect use of a time zone abbreviation:
 *	1.	They might reference tzname[0] before calling tzset (explicitly
 *		or implicitly).
 *	2.	They might reference tzname[1] before calling tzset (explicitly
 *		or implicitly).
 *	3.	They might reference tzname[1] after setting to a time zone
 *		in which Daylight Saving Time is never observed.
 *	4.	They might reference tzname[0] after setting to a time zone
 *		in which Standard Time is never observed.
 *	5.	They might reference tm.TM_ZONE after calling offtime.
 * What's best to do in the above cases is open to debate;
 * for now, we just set things up so that in any of the five cases
 * WILDABBR is used. Another possibility:	initialize tzname[0] to the
 * string "tzname[0] used before set", and similarly for the other cases.
 * And another: initialize tzname[0] to "ERA", with an explanation in the
 * manual page of what this "time zone abbreviation" means (doing this so
 * that tzname[0] has the "normal" length of three characters).
 *----------
 */
#define WILDABBR	"   "
#endif   /* !defined WILDABBR */

extern char wildabbr[];
extern const char gmt[];

/*
 * The DST rules to use if TZ has no rules and we can't load TZDEFRULES.
 * We default to US rules as of 1999-08-17.
 * POSIX 1003.1 section 8.1.1 says that the default DST rules are
 * implementation dependent; for historical reasons, US rules are a
 * common default.
 */
#define TZDEFRULESTRING ",M4.1.0,M10.5.0"

struct rule
{
	int			r_type;			/* type of rule--see below */
	int			r_day;			/* day number of rule */
	int			r_week;			/* week number of rule */
	int			r_mon;			/* month number of rule */
	long		r_time;			/* transition time of rule */
};

#define JULIAN_DAY		0		/* Jn - Julian day */
#define DAY_OF_YEAR		1		/* n - day of year */
#define MONTH_NTH_DAY_OF_WEEK	2		/* Mm.n.d - month, week, day of week */



extern const int mon_lengths[2][MONSPERYEAR];

extern const int year_lengths[2];












/*
src/include/miscadmin.h
*/

/*
 * Date/Time Configuration
 *
 * DateStyle defines the output formatting choice for date/time types:
 *	USE_POSTGRES_DATES specifies traditional Postgres format
 *	USE_ISO_DATES specifies ISO-compliant format
 *	USE_SQL_DATES specifies Oracle/Ingres-compliant format
 *	USE_GERMAN_DATES specifies German-style dd.mm/yyyy
 *
 * DateOrder defines the field order to be assumed when reading an
 * ambiguous date (anything not in YYYY-MM-DD format, with a four-digit
 * year field first, is taken to be ambiguous):
 *	DATEORDER_YMD specifies field order yy-mm-dd
 *	DATEORDER_DMY specifies field order dd-mm-yy ("European" convention)
 *	DATEORDER_MDY specifies field order mm-dd-yy ("US" convention)
 *
 * In the Postgres and SQL DateStyles, DateOrder also selects output field
 * order: day comes before month in DMY style, else month comes before day.
 *
 * The user-visible "DateStyle" run-time parameter subsumes both of these.
 */

/* valid DateStyle values */
#define USE_POSTGRES_DATES		0
#define USE_ISO_DATES			1
#define USE_SQL_DATES			2
#define USE_GERMAN_DATES		3
#define USE_XSD_DATES			4

/* valid DateOrder values */
#define DATEORDER_YMD			0
#define DATEORDER_DMY			1
#define DATEORDER_MDY			2

extern int	DateStyle;
extern int	DateOrder;

/*
 * IntervalStyles
 *	 INTSTYLE_POSTGRES			   Like Postgres < 8.4 when DateStyle = 'iso'
 *	 INTSTYLE_POSTGRES_VERBOSE	   Like Postgres < 8.4 when DateStyle != 'iso'
 *	 INTSTYLE_SQL_STANDARD		   SQL standard interval literals
 *	 INTSTYLE_ISO_8601			   ISO-8601-basic formatted intervals
 */
#define INTSTYLE_POSTGRES			0
#define INTSTYLE_POSTGRES_VERBOSE	1
#define INTSTYLE_SQL_STANDARD		2
#define INTSTYLE_ISO_8601			3

extern int	IntervalStyle;

/*
 * HasCTZSet is true if user has set timezone as a numeric offset from UTC.
 * If so, CTimeZone is the timezone offset in seconds (using the Unix-ish
 * sign convention, ie, positive offset is west of UTC, rather than the
 * SQL-ish convention that positive is east of UTC).
 */
extern bool HasCTZSet;
extern int	CTimeZone;

#define MAXTZLEN		10		/* max TZ name len, not counting tr. null */



/*
 * We keep loaded timezones in a hashtable so we don't have to
 * load and parse the TZ definition file every time one is selected.
 * Because we want timezone names to be found case-insensitively,
 * the hash key is the uppercased name of the zone.
 */
typedef struct
{
	/* tznameupper contains the all-upper-case name of the timezone */
	char		tznameupper[TZ_STRLEN_MAX + 1];
	pg_tz		tz;
} pg_tz_cache;

#include "hsearch.h"

extern HTAB *timezone_cache;





/*

src/include/utils/hsearch.h
*/











#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif


/*******************************************************************************
********************************************************************************
**  src/include/pg_config.h  ***************************************************
********************************************************************************
*******************************************************************************/

/* Define as the maximum alignment requirement of any C data type. */
#define MAXIMUM_ALIGNOF 4

/* Define bytes to use libc memset(). */
#define MEMSET_LOOP_LIMIT 1024

/* Define to 1 if you want 64-bit integer timestamp and interval support.
   (--enable-integer-datetimes) */
#define USE_INTEGER_DATETIMES 1




/* c.h */

#define is_digit(c) ((unsigned)(c) - '0' <= 9)

/*
 * offsetof
 *		Offset of a structure/union field within that structure/union.
 *
 *		XXX This is supposed to be part of stddef.h, but isn't on
 *		some systems (like SunOS 4).
 */
#ifndef offsetof
#define offsetof(type, field)	((long) &((type *)0)->field)
#endif   /* offsetof */

/*
 * lengthof
 *		Number of elements in an array.
 */
#define lengthof(array) (sizeof (array) / sizeof ((array)[0]))

/*
 * endof
 *		Address of the element one past the last in an array.
 */
#define endof(array)	(&(array)[lengthof(array)])



#define TYPEALIGN(ALIGNVAL,LEN)  \
	(((intptr_t) (LEN) + ((ALIGNVAL) - 1)) & ~((intptr_t) ((ALIGNVAL) - 1)))

#define SHORTALIGN(LEN)			TYPEALIGN(ALIGNOF_SHORT, (LEN))
#define INTALIGN(LEN)			TYPEALIGN(ALIGNOF_INT, (LEN))
#define LONGALIGN(LEN)			TYPEALIGN(ALIGNOF_LONG, (LEN))
#define DOUBLEALIGN(LEN)		TYPEALIGN(ALIGNOF_DOUBLE, (LEN))
#define MAXALIGN(LEN)			TYPEALIGN(MAXIMUM_ALIGNOF, (LEN))
/* MAXALIGN covers only built-in types, not buffers */
#define BUFFERALIGN(LEN)		TYPEALIGN(ALIGNOF_BUFFER, (LEN))

#define TYPEALIGN_DOWN(ALIGNVAL,LEN)  \
	(((intptr_t) (LEN)) & ~((intptr_t) ((ALIGNVAL) - 1)))

#define SHORTALIGN_DOWN(LEN)	TYPEALIGN_DOWN(ALIGNOF_SHORT, (LEN))
#define INTALIGN_DOWN(LEN)		TYPEALIGN_DOWN(ALIGNOF_INT, (LEN))
#define LONGALIGN_DOWN(LEN)		TYPEALIGN_DOWN(ALIGNOF_LONG, (LEN))
#define DOUBLEALIGN_DOWN(LEN)	TYPEALIGN_DOWN(ALIGNOF_DOUBLE, (LEN))
#define MAXALIGN_DOWN(LEN)		TYPEALIGN_DOWN(MAXIMUM_ALIGNOF, (LEN))

/* ----------------------------------------------------------------
 *				Section 6:	widely useful macros
 * ----------------------------------------------------------------
 */
/*
 * Max
 *		Return the maximum of two numbers.
 */
#define Max(x, y)		((x) > (y) ? (x) : (y))

/*
 * Min
 *		Return the minimum of two numbers.
 */
#define Min(x, y)		((x) < (y) ? (x) : (y))

/*
 * Abs
 *		Return the absolute value of the argument.
 */
#define Abs(x)			((x) >= 0 ? (x) : -(x))

/*
 * StrNCpy
 *	Like standard library function strncpy(), except that result string
 *	is guaranteed to be null-terminated --- that is, at most N-1 bytes
 *	of the source string will be kept.
 *	Also, the macro returns no result (too hard to do that without
 *	evaluating the arguments multiple times, which seems worse).
 *
 *	BTW: when you need to copy a non-null-terminated string (like a text
 *	datum) and add a null, do not do it with StrNCpy(..., len+1).  That
 *	might seem to work, but it fetches one byte more than there is in the
 *	text object.  One fine day you'll have a SIGSEGV because there isn't
 *	another byte before the end of memory.	Don't laugh, we've had real
 *	live bug reports from real live users over exactly this mistake.
 *	Do it honestly with "memcpy(dst,src,len); dst[len] = '\0';", instead.
 */
#define StrNCpy(dst,src,len) \
	do \
	{ \
		char * _dst = (dst); \
		size_t _len = (len); \
\
		if (_len > 0) \
		{ \
			strncpy(_dst, (src), _len); \
			_dst[_len-1] = '\0'; \
		} \
	} while (0)


/* Get a bit mask of the bits set in non-long aligned addresses */
#define LONG_ALIGN_MASK (sizeof(long) - 1)

/*
 * MemSet
 *	Exactly the same as standard library function memset(), but considerably
 *	faster for zeroing small word-aligned structures (such as parsetree nodes).
 *	This has to be a macro because the main point is to avoid function-call
 *	overhead.	However, we have also found that the loop is faster than
 *	native libc memset() on some platforms, even those with assembler
 *	memset() functions.  More research needs to be done, perhaps with
 *	MEMSET_LOOP_LIMIT tests in configure.
 */
#define MemSet(start, val, len) \
	do \
	{ \
		/* must be void* because we don't know if it is integer aligned yet */ \
		void   *_vstart = (void *) (start); \
		int		_val = (val); \
		size_t	_len = (len); \
\
		if ((((intptr_t) _vstart) & LONG_ALIGN_MASK) == 0 && \
			(_len & LONG_ALIGN_MASK) == 0 && \
			_val == 0 && \
			_len <= MEMSET_LOOP_LIMIT && \
			/* \
			 *	If MEMSET_LOOP_LIMIT == 0, optimizer should find \
			 *	the whole "if" false at compile time. \
			 */ \
			MEMSET_LOOP_LIMIT != 0) \
		{ \
			long *_start = (long *) _vstart; \
			long *_stop = (long *) ((char *) _start + _len); \
			while (_start < _stop) \
				*_start++ = 0; \
		} \
		else \
			memset(_vstart, _val, _len); \
	} while (0)

/*
 * MemSetAligned is the same as MemSet except it omits the test to see if
 * "start" is word-aligned.  This is okay to use if the caller knows a-priori
 * that the pointer is suitably aligned (typically, because he just got it
 * from palloc(), which always delivers a max-aligned pointer).
 */
#define MemSetAligned(start, val, len) \
	do \
	{ \
		long   *_start = (long *) (start); \
		int		_val = (val); \
		size_t	_len = (len); \
\
		if ((_len & LONG_ALIGN_MASK) == 0 && \
			_val == 0 && \
			_len <= MEMSET_LOOP_LIMIT && \
			MEMSET_LOOP_LIMIT != 0) \
		{ \
			long *_stop = (long *) ((char *) _start + _len); \
			while (_start < _stop) \
				*_start++ = 0; \
		} \
		else \
			memset(_start, _val, _len); \
	} while (0)


/*
 * MemSetTest/MemSetLoop are a variant version that allow all the tests in
 * MemSet to be done at compile time in cases where "val" and "len" are
 * constants *and* we know the "start" pointer must be word-aligned.
 * If MemSetTest succeeds, then it is okay to use MemSetLoop, otherwise use
 * MemSetAligned.  Beware of multiple evaluations of the arguments when using
 * this approach.
 */
#define MemSetTest(val, len) \
	( ((len) & LONG_ALIGN_MASK) == 0 && \
	(len) <= MEMSET_LOOP_LIMIT && \
	MEMSET_LOOP_LIMIT != 0 && \
	(val) == 0 )

#define MemSetLoop(start, val, len) \
	do \
	{ \
		long * _start = (long *) (start); \
		long * _stop = (long *) ((char *) _start + (size_t) (len)); \
	\
		while (_start < _stop) \
			*_start++ = 0; \
	} while (0)







/* pgstrcasecmp.c */
/* msb for char */
#define HIGHBIT					(0x80)
#define IS_HIGHBIT_SET(ch)		((unsigned char)(ch) & HIGHBIT)



extern datetkn *timezonetktbl;
extern int	sztimezonetktbl;
extern int	szdatetktbl;
extern int	szdeltatktbl;
extern datetkn deltatktbl[];
extern const datetkn *datecache[MAXDATEFIELDS];
extern const datetkn *deltacache[MAXDATEFIELDS];


extern const datetkn datetktbl[];


/*******************************************************************************
********************************************************************************
**  src/include/utils/date.h  **************************************************
********************************************************************************
*******************************************************************************/

typedef int32_t DateADT;

#ifdef HAVE_INT64_TIMESTAMP
typedef int64_t TimeADT;
#else
typedef double TimeADT;
#endif



typedef struct
{
	TimeADT		time;			/* all time units other than months and years */
	int32_t		zone;			/* numeric time zone, in seconds */
} TimeTzADT;

/*
 * Infinity and minus infinity must be the max and min values of DateADT.
 * We could use INT_MIN and INT_MAX here, but seems better to not assume that
 * int32 == int.
 */
#define DATEVAL_NOBEGIN		((DateADT) (-0x7fffffff - 1))
#define DATEVAL_NOEND		((DateADT) 0x7fffffff)

#define DATE_NOBEGIN(j)		((j) = DATEVAL_NOBEGIN)
#define DATE_IS_NOBEGIN(j)	((j) == DATEVAL_NOBEGIN)
#define DATE_NOEND(j)		((j) = DATEVAL_NOEND)
#define DATE_IS_NOEND(j)	((j) == DATEVAL_NOEND)
#define DATE_NOT_FINITE(j)	(DATE_IS_NOBEGIN(j) || DATE_IS_NOEND(j))

/*
 * Macros for fmgr-callable functions.
 *
 * For TimeADT, we make use of the same support routines as for float8 or int64.
 * Therefore TimeADT is pass-by-reference if and only if float8 or int64 is!
 */
#ifdef HAVE_INT64_TIMESTAMP

#define MAX_TIME_PRECISION 6
#else							/* !HAVE_INT64_TIMESTAMP */

#define MAX_TIME_PRECISION 10

/* round off to MAX_TIME_PRECISION decimal places */
#define TIME_PREC_INV 10000000000.0
#define TIMEROUND(j) (rint(((double) (j)) * TIME_PREC_INV) / TIME_PREC_INV)
#endif   /* HAVE_INT64_TIMESTAMP */




/*******************************************************************************
********************************************************************************
**  function declarations  *****************************************************
********************************************************************************
*******************************************************************************/



int		pg_strcasecmp(const char *, const char *);
int		pg_strncasecmp(const char *, const char *, size_t);
unsigned char	pg_toupper(unsigned char);
unsigned char	pg_tolower(unsigned char);
unsigned char	pg_ascii_toupper(unsigned char);
unsigned char	pg_ascii_tolower(unsigned char);

/* datetime.c */
int		strtoi(const char *, char **, int);

const datetkn *	datebsearch(const char *, const datetkn *, int);

int		ParseDateTime(const char *, char *, size_t, char **, int *, int, int *);
int		DecodeDateTime(char **, int *, int, int *, struct tm *, fsec_t *, int *);
int		DecodeDate(char *, int, int *, bool *, struct tm *);
int		DecodeNumber(int, char *, bool, int, int *, struct tm *, fsec_t *, bool *);
int		DecodeNumberField(int, char *, int, int *, struct tm *, fsec_t *, bool *);
int		ParseFractionalSecond(char *, fsec_t *);
int		DecodeTime(char *, int, int, int *, struct tm *, fsec_t *);
int		DecodeSpecial(int, char *, int *);
int		date2j(int, int, int);
void		j2date(int, int *, int *, int *);
int		ValidateDate(int, bool, bool, bool, struct tm *);
int		DecodeTimezone(char *, int *);
int		pg_next_dst_boundary(const pg_time_t *, long int *, int *, pg_time_t *, long int *, int *, const pg_tz *);


void		pg_timezone_initialize(void);
pg_tz *		pg_tzset(const char *);

const char *	getzname(const char *);
const char *	getqzname(const char *, int);
const char *	getoffset(const char *, long *);
const char *	getrule(const char *, struct rule *);
const char *	getnum(const char *, int *, int, int);
const char *	getsecs(const char *, long *);
pg_time_t	transtime(pg_time_t, int, const struct rule *, long);
int		tzload(const char *, char *, struct state *, int);
int		tzparse(const char *, struct state *, int);
int		typesequiv(const struct state *, int, int);
int		differ_by_repeat(pg_time_t, pg_time_t);
int		pg_open_tzfile(const char *, char *);

const char *	pg_TZDIR(void);
bool		scan_directory_ci(const char *, const char *, int, char *, int);
long		detzcode(const char *);
pg_time_t	detzcode64(const char *);

uint32_t	hash_any(register const unsigned char *, register int);

void		dt2time(Timestamp, int *, int *, int *, fsec_t *);

struct tm *	localsub(const pg_time_t *, long, struct tm *, const pg_tz *);

struct tm *	timesub(const pg_time_t *, long, const struct state *, struct tm *);

int		leaps_thru_end_of(const int);

int		timestamp2tm(Timestamp, int *, struct tm *, fsec_t *, const char **, pg_tz *);
bool		init_timezone_hashtable(void);
int		increment_overflow(int *, int);
struct tm *	pg_localtime(const pg_time_t *, const pg_tz *);

int		tm2timestamp(struct tm *, fsec_t, int *, Timestamp *);

void		gmtload(struct state *);
Timestamp	SetEpochTimestamp(void);

void		GetCurrentDateTime(struct tm *);

void		GetCurrentTimeUsec(struct tm *, fsec_t *, int *);


TimestampTz	timestamptz_in(char *);
void		EncodeTimezone(char *, int, int);
void		AppendTimestampSeconds(char *, struct tm *, fsec_t);
void		AppendSeconds(char *, int, fsec_t, int, bool);
void		TrimTrailingZeros(char *);
int		j2day(int);
char *		timestamptz_out(TimestampTz);
void		EncodeDateTime(struct tm *, fsec_t, bool, int, const char *, int, char *);
void		EncodeSpecialTimestamp(Timestamp, char *);
void		AdjustTimestampForTypmod(Timestamp *, int32_t);
int		DetermineTimeZoneOffset(struct tm *, pg_tz *);
void		DateTimeParseError(int, const char *, const char *);
void		GetEpochTime(struct tm *);
Timestamp	dt2local(Timestamp, int);
struct tm *	gmtsub(const pg_time_t *, long, struct tm *);
struct tm *	pg_gmtime(const pg_time_t *);
TimeOffset	time2t(const int, const int, const int, const fsec_t);
Timestamp	timestamp_in(char *);
char *		timestamp_out(Timestamp);

int		DecodeInterval(char **, int *, int, int, int *, struct tm *, fsec_t *);
int		tm2interval(struct tm *, fsec_t, Interval *);
int		DecodeISO8601Interval(char *, int *, struct tm *, fsec_t *);
inline void	ClearPgTm(struct tm *, fsec_t *);
void		AdjustFractDays(double, struct tm *, fsec_t *, int);
void		AdjustFractSeconds(double, struct tm *, fsec_t *, int);
int		ParseISO8601Number(char *, char **, int *, double *);
int		ISO8601IntegerWidth(char *);
int		interval2tm(Interval, struct tm *, fsec_t *);
void		EncodeInterval(struct tm *, fsec_t, int, char *);
int		DecodeUnits(int, char *, int *);
void		AdjustIntervalForTypmod(Interval *, int32_t);

char *		AddPostgresIntPart(char *, int, const char *, bool *, bool *);
char *		AddISO8601IntPart(char *, int, char);
char *		AddVerboseIntPart(char *, int, const char *, bool *, bool *);

Interval *	interval_in(char *);
char *		interval_out(Interval *);


DateADT		date_in(char *);
char *		date_out(DateADT);

void		EncodeSpecialDate(DateADT, char *);
void		EncodeDateOnly(struct tm *, int, char *);
int		time2tm(TimeADT, struct tm *, fsec_t *);
int		tm2time(struct tm *, fsec_t, TimeADT *);

void		AdjustTimeForTypmod(TimeADT *, int32_t);
int		DecodeTimeOnly(char **, int *, int, int *, struct tm *, fsec_t *, int *);
void		EncodeTimeOnly(struct tm *, fsec_t, bool, int, int, char *);
bool		pg_get_timezone_offset(const pg_tz *, long int *);

char *		time_out(TimeADT);
TimeADT		time_in(char *);



TimeTzADT *	timetz_in(char *);
char *		timetz_out(TimeTzADT *);
int		tm2timetz(struct tm *, fsec_t, int, TimeTzADT *);
int		timetz2tm(TimeTzADT *, struct tm *, fsec_t *, int *);


TimestampTz	GetCurrentTimestamp(void);

Interval *	interval_justify_interval(Interval *);


#endif	/* __DATIZO_H__ */
