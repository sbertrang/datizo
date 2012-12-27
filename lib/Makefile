
LIB =		datizo

SRCS +=		datizo.c
SRCS +=		datebsearch.c
SRCS +=		ParseDateTime.c
SRCS +=		DecodeNumberField.c
SRCS +=		ParseFractionalSecond.c
SRCS +=		DecodeNumber.c
SRCS +=		DecodeSpecial.c
SRCS +=		DecodeDate.c
SRCS +=		DecodeTime.c
SRCS +=		date2j.c
SRCS +=		j2date.c
SRCS +=		ValidateDate.c
SRCS +=		DecodeTimezone.c
SRCS +=		pg_next_dst_boundary.c
SRCS +=		DetermineTimeZoneOffset.c
SRCS +=		dynahash.c
SRCS +=		dt2time.c
SRCS +=		getqzname.c
SRCS +=		getzname.c
SRCS +=		getoffset.c
SRCS +=		getrule.c
SRCS +=		getnum.c
SRCS +=		getsecs.c
SRCS +=		transtime.c
SRCS +=		tzparse.c
SRCS +=		tzload.c
SRCS +=		typesequiv.c
SRCS +=		differ_by_repeat.c
SRCS +=		pg_open_tzfile.c
SRCS +=		pg_TZDIR.c
SRCS +=		scan_directory_ci.c
SRCS +=		detzcode.c
SRCS +=		detzcode64.c
SRCS +=		bitmapset.c
SRCS +=		hashfn.c
SRCS +=		hashfunc.c
SRCS +=		init_timezone_hashtable.c
SRCS +=		GetCurrentTimeUsec.c
SRCS +=		GetCurrentDateTime.c
SRCS +=		timestamp2tm.c
SRCS +=		localsub.c
SRCS +=		timesub.c
SRCS +=		increment_overflow.c
SRCS +=		leaps_thru_end_of.c
SRCS +=		GetCurrentTransactionStartTimestamp.c
SRCS +=		tm2timestamp.c
SRCS +=		dt2local.c
SRCS +=		time2t.c
SRCS +=		SetEpochTimestamp.c
SRCS +=		GetEpochTime.c
SRCS +=		pg_gmtime.c
SRCS +=		gmtsub.c
SRCS +=		gmtload.c
SRCS +=		AdjustTimestampForTypmod.c
SRCS +=		DateTimeParseError.c
SRCS +=		pg_localtime.c
SRCS +=		pg_tzset.c
SRCS +=		pg_timezone_initialize.c
SRCS +=		DecodeDateTime.c
SRCS +=		pgstrcasecmp.c
SRCS +=		strtoi.c
SRCS +=		timestamptz_in.c
SRCS +=		timestamp_in.c
SRCS +=		timestamp_out.c
SRCS +=		DecodeInterval.c
SRCS +=		AdjustFractDays.c
SRCS +=		AdjustFractSeconds.c
SRCS +=		ClearPgTm.c
SRCS +=		DecodeUnits.c
SRCS +=		DecodeISO8601Interval.c
SRCS +=		ParseISO8601Number.c
SRCS +=		ISO8601IntegerWidth.c
SRCS +=		tm2interval.c
SRCS +=		interval2tm.c
SRCS +=		EncodeInterval.c
SRCS +=		AddPostgresIntPart.c
SRCS +=		AddISO8601IntPart.c
SRCS +=		AddVerboseIntPart.c
SRCS +=		AdjustIntervalForTypmod.c
SRCS +=		interval_in.c
SRCS +=		interval_out.c
SRCS +=		EncodeTimezone.c
SRCS +=		AppendSeconds.c
SRCS +=		TrimTrailingZeros.c
SRCS +=		AppendTimestampSeconds.c
SRCS +=		j2day.c
SRCS +=		EncodeSpecialTimestamp.c
SRCS +=		EncodeDateTime.c
SRCS +=		timestamptz_out.c

# date
SRCS +=		date_in.c
SRCS +=		date_out.c
SRCS +=		EncodeDateOnly.c
SRCS +=		EncodeSpecialDate.c

# time
SRCS +=		time_in.c
SRCS +=		time_out.c
SRCS +=		time2tm.c
SRCS +=		tm2time.c
SRCS +=		AdjustTimeForTypmod.c
SRCS +=		DecodeTimeOnly.c
SRCS +=		EncodeTimeOnly.c
SRCS +=		pg_get_timezone_offset.c

# timetz
SRCS +=		timetz_in.c
SRCS +=		timetz_out.c
SRCS +=		tm2timetz.c
SRCS +=		timetz2tm.c

SRCS +=		GetCurrentTimestamp.c

SRCS +=		timestamptz_pl_interval.c
SRCS +=		timestamptz_mi_interval.c
SRCS +=		interval_justify_interval.c
SRCS +=		interval_justify_hours.c
SRCS +=		interval_justify_days.c

SRCS +=		timestamp_cmp_internal.c

SHLIB_MAJOR =	0
SHLIB_MINOR =	0

CPPFLAGS +=	-I${.CURDIR}
LDADD +=	-lm

DISTNAME =	${LIB}-${SHLIB_MAJOR}.${SHLIB_MINOR}
DISTFILE =	${DISTNAME}.tar.gz 

release: ${DISTFILE}

${DISTFILE}:: Makefile
	tar -czvf ${DISTFILE} -s '!^!${DISTNAME}/!p' Makefile *.h ${SRCS}

.include <bsd.lib.mk>
