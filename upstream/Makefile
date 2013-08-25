
DISTURL =	http://ftp.postgresql.org/pub/source/v9.2.2/postgresql-9.2.2.tar.bz2
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

