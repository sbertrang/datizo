
DISTURL =	http://ftp.postgresql.org/pub/source/v9.3rc1/postgresql-9.3rc1.tar.bz2
DISTFILE =	${DISTURL:T}
DISTNAME =	${DISTFILE:R:R}
DISTDIR =	${DISTNAME}

all: extract
	@echo "distdir: ${DISTDIR}"

extract: ${DISTDIR}/.extract

fetch: ${DISTFILE}

${DISTDIR}/.extract: ${DISTFILE}
	tar xjf ${DISTFILE}
	touch ${DISTDIR}/.extract

${DISTFILE}:
	ftp ${DISTURL}

timezonesets:
	mkdir -p ../timezonesets
	pax -vrw -s '!^${DISTDIR}/src/timezone!!' \
	    ${DISTDIR}/src/timezone/tznames \
	    ../timezonesets

