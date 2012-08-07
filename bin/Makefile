
PROG =		datizo

SRCS +=		main.c

MAN =

LIB_DIR =	../lib
INC_DEF =	-I${LIB_DIR}
LIB_DEF =	-L${LIB_DIR}

LDADD +=	${LIB_DEF} -ldatizo

UNAME !=	uname -s

# Darwin
.if ${UNAME} == "Darwin"
CPPVAR =	COPTS
LDVAR =		DYLD_LIBRARY_PATH
# OpenBSD
.else
CPPVAR =	CPPFLAGS
LDVAR =		LD_LIBRARY_PATH
.endif

# dynamic variable due to different behaviours
${CPPVAR} +=	${INC_DEF}

run: ${PROG}
	${LDVAR}=${LIB_DIR} ./${PROG}

debug: ${PROG} ${PROG}.core
	${LDVAR}=${LIB_DIR} gdb ./${PROG} ./${PROG}.core

rebug: ${PROG}
	${LDVAR}=${LIB_DIR} gdb ./${PROG}

.include <bsd.prog.mk>
